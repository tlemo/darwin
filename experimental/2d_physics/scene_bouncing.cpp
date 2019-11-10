
#include "scene_bouncing.h"

#include <core/global_initializer.h>

namespace bouncing_scene {

GLOBAL_INITIALIZER {
  scenesRegistry().add<Factory>("Bouncing Objects");
}

Scene::Scene() : sim::Scene(b2Vec2(0, -9.8f), sim::Rect(-100, -100, 200, 200)) {
  b2BodyDef ground_body_def;
  ground_body_def.position.Set(0.0f, -10.0f);
  ground_body_def.type = b2_staticBody;
  auto ground_body = world_.CreateBody(&ground_body_def);

  b2PolygonShape ground_box;
  ground_box.SetAsBox(100.0f, 10.0f);

  ground_body->CreateFixture(&ground_box, 0.0f);

  sim::addBall(-80, 90, 1, &world_);
  sim::addBall(-70, 80, 2, &world_);
  sim::addBall(-60, 70, 3, &world_);

  auto ball4 = sim::addBall(-50, 60, 5, &world_);
  ball4->ApplyAngularImpulse(500.0f, true);

  sim::addBox(0, 50.0f, 1, 1, &world_);
  sim::addBox(10, 50.0f, 1, 5, &world_);

  auto box3 = sim::addBox(20, 50.0f, 5, 1, &world_);
  box3->ApplyAngularImpulse(1.0f, true);
}

}  // namespace bouncing_scene
