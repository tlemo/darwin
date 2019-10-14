
#include "scene_electric_fence.h"

#include <core/global_initializer.h>

namespace electric_fence_scene {

GLOBAL_INITIALIZER {
  scenesRegistry().add<Factory>("Electric Fence");
}

Scene::Scene() : physics::Scene(b2Vec2(0, -9.8f), physics::Rect(-100, -100, 200, 200)) {
  constexpr float kOuterBoxSize = 100.0f;
  constexpr float kInnerBoxSize = 99.0f;

  b2BodyDef body_def;
  b2EdgeShape shape;

  // outer box
  fence_ = world_.CreateBody(&body_def);

  shape.Set(b2Vec2(-kOuterBoxSize, -kOuterBoxSize),
            b2Vec2(kOuterBoxSize, -kOuterBoxSize));
  fence_->CreateFixture(&shape, 0.0f);

  shape.Set(b2Vec2(kOuterBoxSize, -kOuterBoxSize), b2Vec2(kOuterBoxSize, kOuterBoxSize));
  fence_->CreateFixture(&shape, 0.0f);

  shape.Set(b2Vec2(kOuterBoxSize, kOuterBoxSize), b2Vec2(-kOuterBoxSize, kOuterBoxSize));
  fence_->CreateFixture(&shape, 0.0f);

  shape.Set(b2Vec2(-kOuterBoxSize, kOuterBoxSize),
            b2Vec2(-kOuterBoxSize, -kOuterBoxSize));
  fence_->CreateFixture(&shape, 0.0f);

  // inner box
  auto inner_box = world_.CreateBody(&body_def);

  shape.Set(b2Vec2(kInnerBoxSize, -kInnerBoxSize), b2Vec2(kInnerBoxSize, kInnerBoxSize));
  inner_box->CreateFixture(&shape, 0.0f);

  shape.Set(b2Vec2(kInnerBoxSize, kInnerBoxSize), b2Vec2(-kInnerBoxSize, kInnerBoxSize));
  inner_box->CreateFixture(&shape, 0.0f);

  shape.Set(b2Vec2(-kInnerBoxSize, kInnerBoxSize),
            b2Vec2(-kInnerBoxSize, -kInnerBoxSize));
  inner_box->CreateFixture(&shape, 0.0f);

  // reaper (bottom of the inner box)
  reaper_ = world_.CreateBody(&body_def);
  shape.Set(b2Vec2(-kInnerBoxSize, -kInnerBoxSize),
            b2Vec2(kInnerBoxSize, -kInnerBoxSize));
  reaper_->CreateFixture(&shape, 0.0f);

  // agitator (top)
  auto top_agitator = physics::addCross(0, 0, 45, 2, &world_);

  b2RevoluteJointDef top_jd;
  top_jd.bodyA = inner_box;
  top_jd.bodyB = top_agitator;
  top_jd.localAnchorA.Set(0.0f, 50.0f);
  top_jd.localAnchorB.Set(0.0f, 0.0f);
  top_jd.referenceAngle = 0.0f;
  top_jd.motorSpeed = -0.5f * b2_pi;
  top_jd.maxMotorTorque = 1e8f;
  top_jd.enableMotor = true;
  world_.CreateJoint(&top_jd);

  // agitator (bottom/left)
  auto bottom_left_agitator = physics::addCross(0, 0, 45, 2, &world_);

  b2RevoluteJointDef bottom_left_jd;
  bottom_left_jd.bodyA = inner_box;
  bottom_left_jd.bodyB = bottom_left_agitator;
  bottom_left_jd.localAnchorA.Set(-50.0f, -50.0f);
  bottom_left_jd.localAnchorB.Set(0.0f, 0.0f);
  bottom_left_jd.referenceAngle = 0.0f;
  bottom_left_jd.motorSpeed = 0.8f * b2_pi;
  bottom_left_jd.maxMotorTorque = 1e8f;
  bottom_left_jd.enableMotor = true;
  world_.CreateJoint(&bottom_left_jd);

  // agitator (bottom/right)
  auto bottom_right_agitator = physics::addCross(0, 0, 45, 2, &world_);

  b2RevoluteJointDef bottom_right_jd;
  bottom_right_jd.bodyA = inner_box;
  bottom_right_jd.bodyB = bottom_right_agitator;
  bottom_right_jd.localAnchorA.Set(50.0f, -50.0f);
  bottom_right_jd.localAnchorB.Set(0.0f, 0.0f);
  bottom_right_jd.referenceAngle = 0.0f;
  bottom_right_jd.motorSpeed = -0.8f * b2_pi;
  bottom_right_jd.maxMotorTorque = 1e8f;
  bottom_right_jd.enableMotor = true;
  world_.CreateJoint(&bottom_right_jd);

  // script (shooting projectiles)
  for (int i = 0; i < 500; ++i) {
    script_.record(i / 30.0f, [&, i](float) {
      physics::addBullet(0, 0, 100 * (i % 2 ? 100 : -100), (i - 250) * 50, &world_);
    });
  }
}

void Scene::postStep(float /*dt*/) {
  for (auto body : reaped_bodies_) {
    world_.DestroyBody(body);
  }
  reaped_bodies_.clear();
}

void Scene::onContact(b2Contact* contact) {
  if (contact->IsTouching()) {
    const auto body_a = contact->GetFixtureA()->GetBody();
    const auto body_b = contact->GetFixtureB()->GetBody();
    CHECK(body_a != fence_);
    CHECK(body_b != fence_);

    if (body_a == reaper_)
      reaped_bodies_.insert(body_b);
    else if (body_b == reaper_)
      reaped_bodies_.insert(body_a);
  }
}

}  // namespace electric_fence_scene
