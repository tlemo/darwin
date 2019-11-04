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

#include <math.h>

namespace drone_vision {

Scene::Scene(const b2Vec2& target_velocity, const DroneVision* domain)
    : physics::Scene(b2Vec2(0, 0), physics::Rect(-10, -10, 20, 20)), domain_(domain) {
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

  wall_shape.Set(b2Vec2(-10, -10), b2Vec2(10, -10));
  walls->CreateFixture(&wall_fixture_def);
  wall_shape.Set(b2Vec2(-10, -10), b2Vec2(-10, 10));
  walls->CreateFixture(&wall_fixture_def);
  wall_shape.Set(b2Vec2(10, -10), b2Vec2(10, 10));
  walls->CreateFixture(&wall_fixture_def);
  wall_shape.Set(b2Vec2(-10, 10), b2Vec2(10, 10));
  walls->CreateFixture(&wall_fixture_def);

  // drone & target
  drone_ = createDrone(b2Vec2(0, 0), config.drone_radius);
  target_ = createTarget(b2Vec2(0, 5), target_velocity, config.target_radius);

  // lights
  createLight(walls, b2Vec2(-9, -9), b2Color(1, 1, 1));
  createLight(walls, b2Vec2(9, 9), b2Color(1, 1, 1));

  // sensors
  camera_ = make_unique<Camera>(
      drone_, config.camera_fov, 0.1f, 30.0f, config.camera_resolution);
}

void Scene::postStep(float /*dt*/) {
  fitness_ += fmaxf(cos(aimAngle()), 0);
  updateVariables();
}

void Scene::moveDrone(b2Vec2 force) {
  // limit the magnitude of the force
  const float max_value = domain_->config().max_move_force;
  CHECK(max_value >= 0);
  force.x = fminf(force.x, max_value);
  force.x = fmaxf(force.x, -max_value);
  force.y = fminf(force.y, max_value);
  force.y = fmaxf(force.y, -max_value);

  drone_->ApplyForceToCenter(drone_->GetWorldVector(force), true);
}

void Scene::rotateDrone(float torque) {
  const auto& config = domain_->config();

  // limit the torque
  if (torque < -config.max_rotate_torque) {
    torque = -config.max_rotate_torque;
  } else if (torque > config.max_rotate_torque) {
    torque = config.max_rotate_torque;
  }

  drone_->ApplyTorque(torque, true);
}

float Scene::aimAngle() const {
  const auto global_target_pos = target_->GetWorldPoint(b2Vec2(0, 0));
  const auto local_target_pos = drone_->GetLocalPoint(global_target_pos);

  // drone direction is "up", so atan2 arguments are intentionally (x, y)
  return atan2f(local_target_pos.x, local_target_pos.y);
}

b2Body* Scene::createDrone(const b2Vec2& pos, float radius) {
  b2BodyDef drone_body_def;
  drone_body_def.type = b2_dynamicBody;
  drone_body_def.position = pos;
  drone_body_def.linearDamping = 10.0f;
  drone_body_def.angularDamping = 10.0f;
  auto body = world_.CreateBody(&drone_body_def);

  b2CircleShape drone_shape;
  drone_shape.m_radius = radius;

  b2FixtureDef drone_fixture_def;
  drone_fixture_def.shape = &drone_shape;
  drone_fixture_def.density = 0.1f;
  drone_fixture_def.friction = 1.0f;
  drone_fixture_def.restitution = 0.2f;
  drone_fixture_def.material.color = b2Color(0, 0, 1);
  drone_fixture_def.material.emit_intensity = 0.5f;
  body->CreateFixture(&drone_fixture_def);

  return body;
}

b2Body* Scene::createTarget(const b2Vec2& pos, const b2Vec2& v, float radius) {
  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  body_def.position = pos;
  body_def.linearVelocity = v;
  body_def.linearDamping = 0.0f;
  body_def.angularDamping = 0.0f;
  auto body = world_.CreateBody(&body_def);

  b2CircleShape shape;
  shape.m_radius = radius;

  b2FixtureDef fixture_def;
  fixture_def.shape = &shape;
  fixture_def.density = 0.02f;
  fixture_def.friction = 0.0f;
  fixture_def.restitution = 1.0f;
  fixture_def.material.color = b2Color(1, 0, 0);
  fixture_def.material.shininess = 10;
  fixture_def.material.emit_intensity = 0.1f;
  body->CreateFixture(&fixture_def);

  return body;
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
  variables_.drone_x = drone_->GetPosition().x;
  variables_.drone_y = drone_->GetPosition().y;
  variables_.drone_vx = drone_->GetLinearVelocity().x;
  variables_.drone_vy = drone_->GetLinearVelocity().y;
  variables_.drone_dir = drone_->GetAngle();
  variables_.target_angle = aimAngle();
}

}  // namespace drone_vision
