
#include "scene2.h"

#include <core/global_initializer.h>

namespace sandbox_scene_2 {

GLOBAL_INITIALIZER {
  scenesRegistry().add<Factory>("Scene2");
}

Scene::Scene() : phys::Scene(b2Vec2(0, -9.8f), phys::Rect(-100, -100, 200, 200)) {
  b2BodyDef ground_def;
  auto ground = world_.CreateBody(&ground_def);

  b2EdgeShape shape;
  shape.Set(b2Vec2(-100, -100), b2Vec2(100, -100));
  ground->CreateFixture(&shape, 0.0f);
  shape.Set(b2Vec2(-100, -100), b2Vec2(-100, 100));
  ground->CreateFixture(&shape, 0.0f);
  shape.Set(b2Vec2(100, -100), b2Vec2(100, 100));
  ground->CreateFixture(&shape, 0.0f);
  shape.Set(b2Vec2(-100, 100), b2Vec2(100, 100));
  ground->CreateFixture(&shape, 0.0f);

  auto box = phys::addBox(0, 0, 20, 2, &world_);

  b2RevoluteJointDef jd;
  jd.bodyA = ground;
  jd.bodyB = box;
  jd.localAnchorA.Set(0.0f, 0.0f);
  jd.localAnchorB.Set(-20.0f, 0.0f);
  jd.referenceAngle = 0.0f;
  jd.motorSpeed = 0.5f * b2_pi;
  jd.maxMotorTorque = 1e8f;
  jd.enableMotor = true;
  world_.CreateJoint(&jd);

  auto spin_box = phys::addBox(0, 0, 10, 2, &world_);

  b2RevoluteJointDef hinge_def;
  hinge_def.bodyA = box;
  hinge_def.bodyB = spin_box;
  hinge_def.localAnchorA.Set(20.0f, 0.0f);
  hinge_def.localAnchorB.Set(-10.0f, 0.0f);
  world_.CreateJoint(&hinge_def);

  auto spin_box_2 = phys::addBox(0, 0, 10, 2, &world_);

  b2RevoluteJointDef hinge_def_2;
  hinge_def_2.bodyA = spin_box;
  hinge_def_2.bodyB = spin_box_2;
  hinge_def_2.localAnchorA.Set(10.0f, 0.0f);
  hinge_def_2.localAnchorB.Set(-10.0f, 0.0f);
  world_.CreateJoint(&hinge_def_2);

  for (int i = 0; i < 100; ++i) {
    script_.record(i, [&](float) { phys::addBullet(-95, 5, 100 * 100, 0, &world_); });
  }

  script_.record(1.5f, [&](float) { phys::addBox(0, 50, 30, 1, &world_); });

  script_.record(5.5f, [&](float) {
    auto box = phys::addBox(0, 50, 10, 1, &world_);
    box->ApplyAngularImpulse(1000.0f, true);
  });
}

}  // namespace sandbox_scene_2
