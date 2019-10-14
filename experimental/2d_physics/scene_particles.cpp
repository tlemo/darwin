
#include "scene_particles.h"

#include <core/global_initializer.h>

namespace particles_scene {

GLOBAL_INITIALIZER {
  scenesRegistry().add<Factory>("Particles");
}

Scene::Scene() : physics::Scene(b2Vec2(0, -9.8f), physics::Rect(-100, -100, 200, 200)) {
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

  auto box = physics::addBox(0, 0, 20, 2, &world_);

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

  auto middle = physics::addBox(0, 0, 10, 2, &world_);

  b2RevoluteJointDef hinge_def;
  hinge_def.bodyA = box;
  hinge_def.bodyB = middle;
  hinge_def.localAnchorA.Set(20.0f, 0.0f);
  hinge_def.localAnchorB.Set(-10.0f, 0.0f);
  world_.CreateJoint(&hinge_def);

  auto cross = physics::addCross(0, 0, 15, 2, &world_);

  b2RevoluteJointDef hinge_def_2;
  hinge_def_2.bodyA = middle;
  hinge_def_2.bodyB = cross;
  hinge_def_2.localAnchorA.Set(10.0f, 0.0f);
  hinge_def_2.localAnchorB.Set(-15.0f, 0.0f);
  world_.CreateJoint(&hinge_def_2);

  for (int i = 0; i < 500; ++i) {
    script_.record(i * 0.25f, [&](float) {
      physics::addBullet(-95, 5, 100 * 100, 0, &world_);
      physics::addBullet(95, 5, -100 * 100, 0, &world_);
    });
  }

  script_.record(2.5f, [&](float) { physics::addBox(0, 50, 30, 1, &world_); });

  script_.record(5.0f, [&](float) {
    physics::addBall(-95, 95, 1, &world_);
    physics::addBall(-95, -95, 2, &world_);
    physics::addBall(95, 95, 3, &world_);
    physics::addBall(95, -95, 4, &world_);
  });

  script_.record(7.5f, [&](float) {
    auto box = physics::addBox(0, 50, 10, 1, &world_);
    box->ApplyAngularImpulse(1000.0f, true);
  });
}

}  // namespace particles_scene
