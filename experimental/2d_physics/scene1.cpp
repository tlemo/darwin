
#include "scene1.h"

#include <core/global_initializer.h>

namespace sandbox_scene_1 {

GLOBAL_INITIALIZER {
  scenesRegistry().add<Factory>("Scene1");
}

Scene::Scene() : phys::Scene(b2Vec2(0, -9.8f), phys::Rect(-100, -100, 200, 200)) {
  b2BodyDef ground_body_def;
  ground_body_def.position.Set(0.0f, -10.0f);
  ground_body_def.type = b2_staticBody;
  auto ground_body = world_.CreateBody(&ground_body_def);

  b2PolygonShape ground_box;
  ground_box.SetAsBox(100.0f, 10.0f);

  ground_body->CreateFixture(&ground_box, 0.0f);

  phys::addBall(-80, 90, 1, &world_);
  phys::addBall(-70, 80, 2, &world_);
  phys::addBall(-60, 70, 3, &world_);

  auto ball4 = phys::addBall(-50, 60, 5, &world_);
  ball4->ApplyAngularImpulse(500.0f, true);

  phys::addBox(0, 50.0f, 1, 1, &world_);
  phys::addBox(10, 50.0f, 1, 5, &world_);

  auto box3 = phys::addBox(20, 50.0f, 5, 1, &world_);
  box3->ApplyAngularImpulse(1.0f, true);
}

}  // namespace sandbox_scene_1
