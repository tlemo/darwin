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

#include "scene.h"

#include <core/math_2d.h>

#include <math.h>

namespace drone_follow {

Scene::Scene(Seed seed, const DroneFollow* domain)
    : sim::Scene(b2Vec2(0, 0), sim::Rect(-kWidth / 2, -kHeight / 2, kWidth, kHeight)),
      rnd_(seed),
      domain_(domain) {
  const auto& config = domain_->config();

  // walls
  b2BodyDef walls_def;
  auto walls = world_.CreateBody(&walls_def);

  b2EdgeShape wall_shape;
  b2FixtureDef wall_fixture_def;
  wall_fixture_def.shape = &wall_shape;
  wall_fixture_def.friction = 1.0f;
  wall_fixture_def.restitution = 0.5f;
  wall_fixture_def.material.color = b2Color(1, 1, 0);
  wall_fixture_def.material.emit_intensity = 0.1f;

  constexpr float kHalfWidth = kWidth / 2;
  constexpr float kHalfHeight = kHeight / 2;
  wall_shape.Set(b2Vec2(-kHalfWidth, -kHalfHeight), b2Vec2(kHalfWidth, -kHalfHeight));
  walls->CreateFixture(&wall_fixture_def);
  wall_shape.Set(b2Vec2(-kHalfWidth, -kHalfHeight), b2Vec2(-kHalfWidth, kHalfHeight));
  walls->CreateFixture(&wall_fixture_def);
  wall_shape.Set(b2Vec2(kHalfWidth, -kHalfHeight), b2Vec2(kHalfWidth, kHalfHeight));
  walls->CreateFixture(&wall_fixture_def);
  wall_shape.Set(b2Vec2(-kHalfWidth, kHalfHeight), b2Vec2(kHalfWidth, kHalfHeight));
  walls->CreateFixture(&wall_fixture_def);

  // drone
  drone_ = make_unique<Drone>(&world_, domain_->droneConfig());

  // target drone
  sim::DroneConfig target_drone_config;
  target_drone_config.position = b2Vec2(0, config.target_distance);
  target_drone_config.radius = config.drone_radius;
  target_drone_config.max_move_force = config.max_move_force;
  target_drone_config.max_rotate_torque = config.max_rotate_torque;
  target_drone_config.color = b2Color(1, 0, 0);
  target_drone_ = make_unique<Drone>(&world_, target_drone_config);

  // lights
  createLight(walls, b2Vec2(-9, -9), b2Color(1, 1, 1));
  createLight(walls, b2Vec2(9, 9), b2Color(1, 1, 1));

  generateTargetPos();
}

void Scene::preStep() {
  const auto& config = target_drone_->config();
  const auto body = target_drone_->body();

  // if the drone got to the target, generate a new target
  const auto dist_squared = (body->GetPosition() - target_pos_).LengthSquared();
  if (dist_squared <= config.radius * config.radius) {
    generateTargetPos();
  }

  // drone direction is "up", so atan2 arguments are intentionally (x, y)
  const auto local_target_pos = body->GetLocalPoint(target_pos_);
  const auto target_angle = atan2(local_target_pos.x, local_target_pos.y);

  // steer
  const float aim_offset = float(target_angle / math::kPi);
  const float torque = -aim_offset * config.max_rotate_torque;
  target_drone_->rotate(torque);

  // move
  if (fabs(target_angle) < math::kPi / 2) {
    const float force = ((cosf(target_angle * 2) + 1) / 2) * config.max_move_force;
    target_drone_->move(b2Vec2(0, force));
  }
}

void Scene::postStep(float dt) {
  drone_->postStep(dt);
  updateFitness();
  updateVariables();
}

void Scene::createLight(b2Body* body, const b2Vec2& pos, const b2Color& color) {
  b2LightDef light_def;
  light_def.body = body;
  light_def.color = color;
  light_def.intensity = 2.0f;
  light_def.attenuation_distance = 25.0f;
  light_def.position = pos;
  world_.CreateLight(&light_def);
}

void Scene::updateVariables() {
  const b2Body* drone_body = drone_->body();
  variables_.drone_x = drone_body->GetPosition().x;
  variables_.drone_y = drone_body->GetPosition().y;
  variables_.drone_vx = drone_body->GetLinearVelocity().x;
  variables_.drone_vy = drone_body->GetLinearVelocity().y;
  variables_.drone_dir = drone_body->GetAngle();
  variables_.target_dist = targetDistance();
}

float Scene::targetDistance() const {
  const auto drone_pos = drone_->body()->GetPosition();
  const auto target_drone_pos = target_drone_->body()->GetPosition();
  return (target_drone_pos - drone_pos).Length();
}

void Scene::updateFitness() {
  const auto dist_from_target = targetDistance();
  const auto ideal_dist = domain_->config().target_distance;
  fitness_ += 1 - fabsf(tanhf(dist_from_target - ideal_dist));
}

void Scene::generateTargetPos() {
  const auto& config = target_drone_->config();
  const float d = config.radius * 2;
  uniform_real_distribution<float> dist_x(-kWidth / 2 + d, kWidth / 2 - d);
  uniform_real_distribution<float> dist_y(-kHeight / 2 + d, kHeight / 2 - d);
  start_pos_ = target_drone_->body()->GetPosition();
  target_pos_ = b2Vec2(dist_x(rnd_), dist_y(rnd_));
}

}  // namespace drone_follow
