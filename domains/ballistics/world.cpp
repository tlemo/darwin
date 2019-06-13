// Copyright 2019 The Darwin Neuroevolution Framework Authors.
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

#include "world.h"

namespace ballistics {

World::World(b2Vec2 target_pos, const Ballistics* domain)
    : b2_world_(b2Vec2(0, -domain->config().gravity)), domain_(domain) {
  const auto& config = domain_->config();
    
  // create target body
  b2BodyDef target_body_def;
  target_body_def.type = b2_staticBody;
  target_body_def.position = target_pos;
  target_ = b2_world_.CreateBody(&target_body_def);

  b2CircleShape target_shape;
  target_shape.m_radius = config.target_radius;

  b2FixtureDef target_fixture_def;
  target_fixture_def.shape = &target_shape;
  target_fixture_def.isSensor = true;
  target_->CreateFixture(&target_fixture_def);
  
  // default vertical limit
  vertical_limit_ = target_pos.y - (config.target_radius + config.projectile_radius);
}

void World::fireProjectile(float aim_angle) {
  CHECK(projectile_ == nullptr);

  // create the projectile
  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  body_def.position.Set(0, 0);
  body_def.bullet = true;
  projectile_ = b2_world_.CreateBody(&body_def);

  b2CircleShape shape;
  shape.m_radius = domain_->config().projectile_radius;

  b2FixtureDef fixture_def;
  fixture_def.shape = &shape;
  fixture_def.density = 1.0f;
  projectile_->CreateFixture(&fixture_def);

  // initial velocity
  const float v = domain_->config().projectile_velocity;
  projectile_->SetLinearVelocity(b2Vec2(v * cos(aim_angle), v * sin(aim_angle)));
}

bool World::simStep() {
  CHECK(projectile_ != nullptr);

  constexpr float32 kTimeStep = 1.0f / 50.0f;
  constexpr int32 kVelocityIterations = 5;
  constexpr int32 kPositionIterations = 5;

  // box2d: simulate one step
  b2_world_.Step(kTimeStep, kVelocityIterations, kPositionIterations);

  const float y = projectile_->GetPosition().y;
  const float vy = projectile_->GetLinearVelocity().y;
  return vy >= 0 || y > vertical_limit_;
}

}  // namespace ballistics
