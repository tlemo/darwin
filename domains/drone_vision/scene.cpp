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
using namespace std;

namespace drone_vision {

Scene::Scene(const b2Vec2& target_velocity, const DroneVision* domain)
    : sim::Scene(b2Vec2(0, 0), sim::Rect(-10, -10, 20, 20)), domain_(domain) {
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

  if (config.scene_columns) {
    createColumns();
  }

  if (config.scene_debris) {
    createDebris();
  }

  // drone
  drone_ = make_unique<Drone>(&world_, domain_->droneConfig());

  // target
  target_ = createTarget(b2Vec2(0, 5), target_velocity, config.target_radius);

  // lights
  createLight(walls, b2Vec2(-9, -9), b2Color(1, 1, 1));
  createLight(walls, b2Vec2(9, 9), b2Color(1, 1, 1));
}

void Scene::postStep(float dt) {
  drone_->postStep(dt);
  updateFitness();
  updateVariables();
}

float Scene::aimAngle() const {
  const auto global_target_pos = target_->GetWorldPoint(b2Vec2(0, 0));
  const auto local_target_pos = drone_->body()->GetLocalPoint(global_target_pos);

  // drone direction is "up", so atan2 arguments are intentionally (x, y)
  return atan2f(local_target_pos.x, local_target_pos.y);
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

void Scene::createDebris() {
  createRoundDebris(b2Vec2(-8, 8), 0.1f);
  createRoundDebris(b2Vec2(-8, -8), 0.1f);
  createRoundDebris(b2Vec2(8, 8), 0.1f);
  createRoundDebris(b2Vec2(8, -8), 0.1f);

  createBoxDebris(b2Vec2(-4, 4), 0.1f, 0.2f);
  createBoxDebris(b2Vec2(-4, -4), 0.2f, 0.1f);
  createBoxDebris(b2Vec2(4, 4), 0.2f, 0.1f);
  createBoxDebris(b2Vec2(4, -4), 0.1f, 0.2f);
}

void Scene::createRoundDebris(const b2Vec2& pos, float radius) {
  uniform_real_distribution<float> dist(-2, 2);

  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  body_def.position = pos;
  body_def.linearVelocity.Set(dist(rnd_), dist(rnd_));
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
  fixture_def.material.color = b2Color(0, 1, 1);
  fixture_def.material.shininess = 10;
  fixture_def.material.emit_intensity = 0.3f;
  body->CreateFixture(&fixture_def);
}

void Scene::createBoxDebris(const b2Vec2& pos, float width, float height) {
  uniform_real_distribution<float> dist_v(-2, 2);
  uniform_real_distribution<float> dist_r(-1, 1);

  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  body_def.position = pos;
  body_def.linearVelocity.Set(dist_v(rnd_), dist_v(rnd_));
  body_def.angularVelocity = dist_r(rnd_);
  body_def.linearDamping = 0.0f;
  body_def.angularDamping = 0.0f;
  auto body = world_.CreateBody(&body_def);

  b2PolygonShape shape;
  shape.SetAsBox(width, height);

  b2FixtureDef fixture_def;
  fixture_def.shape = &shape;
  fixture_def.density = 0.02f;
  fixture_def.friction = 0.0f;
  fixture_def.restitution = 1.0f;
  fixture_def.material.color = b2Color(0, 1, 1);
  fixture_def.material.shininess = 10;
  fixture_def.material.emit_intensity = 0.3f;
  body->CreateFixture(&fixture_def);
}

void Scene::createColumns() {
  b2BodyDef body_def;
  auto body = world_.CreateBody(&body_def);

  const b2Color green(0, 1, 0);
  createColumnFixture(body, b2Vec2(-6, 6), green);
  createColumnFixture(body, b2Vec2(-6, 0), green);
  createColumnFixture(body, b2Vec2(-6, -6), green);
  createColumnFixture(body, b2Vec2(-3, 3), green);
  createColumnFixture(body, b2Vec2(-3, -3), green);

  const b2Color blue(0, 0, 1);
  createColumnFixture(body, b2Vec2(6, 6), blue);
  createColumnFixture(body, b2Vec2(6, 0), blue);
  createColumnFixture(body, b2Vec2(6, -6), blue);
  createColumnFixture(body, b2Vec2(3, 3), blue);
  createColumnFixture(body, b2Vec2(3, -3), blue);
}

void Scene::createColumnFixture(b2Body* body, const b2Vec2& pos, const b2Color& color) {
  b2CircleShape shape;
  shape.m_radius = 0.2f;
  shape.m_p = pos;

  b2FixtureDef fixture_def;
  fixture_def.shape = &shape;
  fixture_def.friction = 1.0f;
  fixture_def.restitution = 0.5f;
  fixture_def.material.color = color;
  fixture_def.material.emit_intensity = 0.2f;
  fixture_def.material.shininess = 5;
  body->CreateFixture(&fixture_def);
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

void Scene::updateFitness() {
  fitness_ += 1 - fabsf(tanhf(2 * aimAngle()));
}

void Scene::updateVariables() {
  const b2Body* drone_body = drone_->body();
  variables_.drone_x = drone_body->GetPosition().x;
  variables_.drone_y = drone_body->GetPosition().y;
  variables_.drone_vx = drone_body->GetLinearVelocity().x;
  variables_.drone_vy = drone_body->GetLinearVelocity().y;
  variables_.drone_dir = drone_body->GetAngle();
  variables_.target_angle = aimAngle();
}

}  // namespace drone_vision
