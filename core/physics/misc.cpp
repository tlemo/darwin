// Copyright The Darwin Neuroevolution Framework Authors.
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

#include "misc.h"

namespace physics {

b2Body* addBall(float x, float y, float r, b2World* world) {
  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  body_def.position.Set(x, y);
  auto body = world->CreateBody(&body_def);

  b2CircleShape shape;
  shape.m_radius = r;

  b2FixtureDef fixture_def;
  fixture_def.shape = &shape;
  fixture_def.density = 1.0f;
  fixture_def.friction = 0.3f;
  fixture_def.restitution = 0.6f;
  body->CreateFixture(&fixture_def);

  return body;
}

b2Body* addBox(float x, float y, float w, float h, b2World* world) {
  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  body_def.position.Set(x, y);
  auto body = world->CreateBody(&body_def);

  b2PolygonShape shape;
  shape.SetAsBox(w, h);

  // Define the dynamic body fixture.
  b2FixtureDef fixture_def;
  fixture_def.shape = &shape;
  fixture_def.density = 1.0f;
  fixture_def.friction = 0.6f;
  fixture_def.restitution = 0.6f;
  body->CreateFixture(&fixture_def);

  return body;
}

b2Body* addCross(float x, float y, float w, float h, b2World* world) {
  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  body_def.position.Set(x, y);
  auto body = world->CreateBody(&body_def);

  b2PolygonShape shape;
  shape.SetAsBox(w, h);

  b2FixtureDef fixture_def;
  fixture_def.shape = &shape;
  fixture_def.density = 1.0f;
  fixture_def.friction = 0.6f;
  fixture_def.restitution = 0.6f;
  body->CreateFixture(&fixture_def);

  shape.SetAsBox(h, w);

  fixture_def.shape = &shape;
  fixture_def.density = 1.0f;
  fixture_def.friction = 0.6f;
  fixture_def.restitution = 0.6f;
  body->CreateFixture(&fixture_def);

  return body;
}

b2Body* addBullet(float x, float y, float dx, float dy, b2World* world) {
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

b2Body* addBoxProjectile(float x, float y, float dx, float dy, b2World* world) {
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
  fixture_def.restitution = 0;
  body->CreateFixture(&fixture_def);

  body->ApplyLinearImpulseToCenter(b2Vec2(dx, dy), true);

  return body;
}

}  // namespace physics
