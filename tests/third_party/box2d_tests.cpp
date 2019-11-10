// Copyright 2018 The Darwin Neuroevolution Framework Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <third_party/box2d/box2d.h>

#include <core/utils.h>
#include <core/sim/script.h>
#include <third_party/gtest/gtest.h>

#include <unordered_set>
using namespace std;

namespace box2d_tests {

// test contact listener: it monitors all the contacts against two reference bodies:
// 1. checkpoint body: expected contacts (the touching bodies are recorded)
// 2. fence body: touching this body indicates a test failure
class ContactListener : public b2ContactListener {
 public:
  explicit ContactListener(const b2Body* checkpoint, const b2Body* fence)
      : checkpoint_(checkpoint), fence_(fence) {}

  void BeginContact(b2Contact* contact) override {
    if (!contact->IsTouching())
      return;

    const auto body_a = contact->GetFixtureA()->GetBody();
    const auto body_b = contact->GetFixtureB()->GetBody();

    EXPECT_NE(body_a, fence_);
    EXPECT_NE(body_b, fence_);

    if (body_a == checkpoint_)
      checkpoint_bodies_.insert(body_b);
    else if (body_b == checkpoint_)
      checkpoint_bodies_.insert(body_a);
  }

  unordered_set<b2Body*> checkpointBodies() {
    unordered_set<b2Body*> tmp;
    tmp.swap(checkpoint_bodies_);
    return tmp;
  }

 private:
  const b2Body* checkpoint_ = nullptr;
  const b2Body* fence_ = nullptr;
  unordered_set<b2Body*> checkpoint_bodies_;
};

b2Body* createRoundProjectile(float x, float y, float dx, float dy, b2World* world) {
  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  body_def.position.Set(x, y);
  body_def.bullet = true;
  auto body = world->CreateBody(&body_def);

  b2CircleShape shape;
  shape.m_radius = 0.3f;

  b2FixtureDef fixture_def;
  fixture_def.shape = &shape;
  fixture_def.density = 100.0f;
  fixture_def.friction = 1.0f;
  fixture_def.restitution = 0.9f;
  body->CreateFixture(&fixture_def);

  body->ApplyLinearImpulseToCenter(b2Vec2(dx, dy), true);
  return body;
}

b2Body* createBoxProjectile(float x, float y, float dx, float dy, b2World* world) {
  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  body_def.position.Set(x, y);
  body_def.bullet = true;
  auto body = world->CreateBody(&body_def);

  b2PolygonShape shape;
  shape.SetAsBox(0.5f, 0.2f);

  b2FixtureDef fixture_def;
  fixture_def.shape = &shape;
  fixture_def.density = 10.0f;
  fixture_def.friction = 0.1f;
  fixture_def.restitution = 0.0f;
  body->CreateFixture(&fixture_def);

  body->ApplyLinearImpulseToCenter(b2Vec2(dx, dy), true);
  return body;
}

TEST(Box2dTest, RoundProjectiles) {
  constexpr float kOuterBoxSize = 100.0f;
  constexpr float kInnerBoxSize = 99.0f;
  constexpr float kGravity = -9.8f;

  b2Vec2 gravity(0.0f, kGravity);
  b2World world(gravity);

  b2BodyDef body_def;
  b2EdgeShape shape;

  // outer box
  auto outer_box = world.CreateBody(&body_def);

  shape.Set(b2Vec2(-kOuterBoxSize, -kOuterBoxSize),
            b2Vec2(kOuterBoxSize, -kOuterBoxSize));
  outer_box->CreateFixture(&shape, 0.0f);

  shape.Set(b2Vec2(kOuterBoxSize, -kOuterBoxSize), b2Vec2(kOuterBoxSize, kOuterBoxSize));
  outer_box->CreateFixture(&shape, 0.0f);

  shape.Set(b2Vec2(kOuterBoxSize, kOuterBoxSize), b2Vec2(-kOuterBoxSize, kOuterBoxSize));
  outer_box->CreateFixture(&shape, 0.0f);

  shape.Set(b2Vec2(-kOuterBoxSize, kOuterBoxSize),
            b2Vec2(-kOuterBoxSize, -kOuterBoxSize));
  outer_box->CreateFixture(&shape, 0.0f);

  // inner box
  auto inner_box = world.CreateBody(&body_def);

  shape.Set(b2Vec2(kInnerBoxSize, -kInnerBoxSize), b2Vec2(kInnerBoxSize, kInnerBoxSize));
  inner_box->CreateFixture(&shape, 0.0f);

  shape.Set(b2Vec2(kInnerBoxSize, kInnerBoxSize), b2Vec2(-kInnerBoxSize, kInnerBoxSize));
  inner_box->CreateFixture(&shape, 0.0f);

  shape.Set(b2Vec2(-kInnerBoxSize, kInnerBoxSize),
            b2Vec2(-kInnerBoxSize, -kInnerBoxSize));
  inner_box->CreateFixture(&shape, 0.0f);

  // checkpoint
  auto checkpoint = world.CreateBody(&body_def);
  shape.Set(b2Vec2(-kInnerBoxSize, -kInnerBoxSize),
            b2Vec2(kInnerBoxSize, -kInnerBoxSize));
  checkpoint->CreateFixture(&shape, 0.0f);

  // contact listener
  ContactListener contact_listener(checkpoint, outer_box);
  world.SetContactListener(&contact_listener);

  // script (shooting projectiles)
  sim::Script script;
  constexpr int kProjectilesCount = 200;
  for (int i = 0; i < kProjectilesCount; ++i) {
    script.record(i / 20.0f, [&, i](float) {
      createRoundProjectile(0, 0, 100 * (i % 2 ? 100 : -100), (i - 50) * 150, &world);
    });
  }

  // run the simulation
  constexpr float32 kTotalTime = 25.0f;  
  constexpr float32 kTimeStep = 1.0f / 50.0f;
  constexpr int32 kVelocityIterations = 5;
  constexpr int32 kPositionIterations = 5;

  script.start();
  
  int checkpoint_objects = 0;
  for (float timestamp = 0; timestamp <= kTotalTime; timestamp += kTimeStep) {
    // script actions
    script.play(timestamp);

    // box2d: simulate one step
    world.Step(kTimeStep, kVelocityIterations, kPositionIterations);

    // remove all the objects which hit the checkpoint
    for (auto body : contact_listener.checkpointBodies()) {
      world.DestroyBody(body);
      ++checkpoint_objects;
    }
  }
  
  EXPECT_EQ(checkpoint_objects, kProjectilesCount);
}

// this test case might be pushing Box2D a bit too hard (apparently it's not too hard to
// get even "bullet" bodies to go though solid edges)
//
// the simulation parameters have been selected conservatively to make the checks
// predictible (and reasonability fast)
//
TEST(Box2dTest, BoxProjectiles) {
  constexpr float kOuterBoxSize = 100.0f;
  constexpr float kInnerBoxSize = 99.0f;
  constexpr float kGravity = -9.8f;

  b2Vec2 gravity(0.0f, kGravity);
  b2World world(gravity);

  b2BodyDef body_def;
  b2EdgeShape shape;

  // outer box
  auto outer_box = world.CreateBody(&body_def);

  shape.Set(b2Vec2(-kOuterBoxSize, -kOuterBoxSize),
            b2Vec2(kOuterBoxSize, -kOuterBoxSize));
  outer_box->CreateFixture(&shape, 0.0f);

  shape.Set(b2Vec2(kOuterBoxSize, -kOuterBoxSize), b2Vec2(kOuterBoxSize, kOuterBoxSize));
  outer_box->CreateFixture(&shape, 0.0f);

  shape.Set(b2Vec2(kOuterBoxSize, kOuterBoxSize), b2Vec2(-kOuterBoxSize, kOuterBoxSize));
  outer_box->CreateFixture(&shape, 0.0f);

  shape.Set(b2Vec2(-kOuterBoxSize, kOuterBoxSize),
            b2Vec2(-kOuterBoxSize, -kOuterBoxSize));
  outer_box->CreateFixture(&shape, 0.0f);

  // barrier
  auto barrier = world.CreateBody(&body_def);

  shape.Set(b2Vec2(-kInnerBoxSize, kInnerBoxSize), b2Vec2(0, kInnerBoxSize));
  barrier->CreateFixture(&shape, 0.0f);

  shape.Set(b2Vec2(0, kInnerBoxSize), b2Vec2(0, -70));
  barrier->CreateFixture(&shape, 0.0f);

  shape.Set(b2Vec2(-kInnerBoxSize, 0), b2Vec2(30, -kInnerBoxSize));
  barrier->CreateFixture(&shape, 0.0f);

  // checkpoint
  auto checkpoint = world.CreateBody(&body_def);
  shape.Set(b2Vec2(30, -kInnerBoxSize), b2Vec2(30, -70));
  checkpoint->CreateFixture(&shape, 0.0f);

  // contact listener
  ContactListener contact_listener(checkpoint, outer_box);
  world.SetContactListener(&contact_listener);

  // script (shooting projectiles)
  sim::Script script;
  constexpr int kProjectilesCount = 50;
  for (int i = 0; i < kProjectilesCount; ++i) {
    script.record(i / 10.0f, [&, i](float) {
      auto projectile = createBoxProjectile(-95, 50, 100 * 100, (i - 25) * 1000, &world);
      projectile->ApplyAngularImpulse(i, true);
      projectile->SetLinearDamping(0.2f);
      projectile->SetAngularDamping(0.2f);
    });
  }

  // run the simulation
  constexpr float32 kTotalTime = 50.0f;  
  constexpr float32 kTimeStep = 1.0f / 30.0f;
  constexpr int32 kVelocityIterations = 10;
  constexpr int32 kPositionIterations = 10;

  script.start();
  
  int checkpoint_objects = 0;
  for (float timestamp = 0; timestamp <= kTotalTime; timestamp += kTimeStep) {
    // script actions
    script.play(timestamp);

    // box2d: simulate one step
    world.Step(kTimeStep, kVelocityIterations, kPositionIterations);

    // remove all the objects which hit the checkpoint
    for (auto body : contact_listener.checkpointBodies()) {
      world.DestroyBody(body);
      ++checkpoint_objects;
    }
  }
  
  EXPECT_EQ(checkpoint_objects, kProjectilesCount);
}

}  // namespace box2d_tests
