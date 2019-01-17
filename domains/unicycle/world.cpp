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

#include <core/math_2d.h>

namespace unicycle {

b2Body* World::createGround(float target_position) {
  const auto& config = domain_->config();

  b2BodyDef ground_body_def;
  ground_body_def.position.Set(0, kGroundY);
  auto ground = b2_world_.CreateBody(&ground_body_def);

  b2EdgeShape ground_shape;
  ground_shape.Set(b2Vec2(-config.max_distance, 0), b2Vec2(config.max_distance, 0));

  b2FixtureDef ground_fixture_def;
  ground_fixture_def.shape = &ground_shape;
  ground_fixture_def.density = 0;
  ground_fixture_def.friction = kGroundFriction;
  ground->CreateFixture(&ground_fixture_def);

  ground_shape.Set(b2Vec2(target_position, 0), b2Vec2(target_position, -kGroundY));
  ground->CreateFixture(&ground_shape, 0.0f);

  return ground;
}

b2Body* World::createPole(float initial_angle) {
  const auto& config = domain_->config();
  const float wheel_axle_height = config.wheel_radius + kGroundY;
  const float pole_half_height = config.pole_length / 2;

  b2BodyDef pole_body_def;
  pole_body_def.type = b2_dynamicBody;
  pole_body_def.position.Set(0.0f, wheel_axle_height);
  pole_body_def.angle = math::degreesToRadians(initial_angle);
  auto pole = b2_world_.CreateBody(&pole_body_def);

  b2PolygonShape pole_shape;
  pole_shape.SetAsBox(
      kPoleHalfWidth, pole_half_height, b2Vec2(0, pole_half_height), 0.0f);

  b2FixtureDef pole_fixture_def;
  pole_fixture_def.shape = &pole_shape;
  pole_fixture_def.density = config.pole_density;
  pole_fixture_def.filter.groupIndex = -1;
  pole->CreateFixture(&pole_fixture_def);

  return pole;
}

b2Body* World::createWheel() {
  const auto& config = domain_->config();

  b2BodyDef wheel_body_def;
  wheel_body_def.type = b2_dynamicBody;
  wheel_body_def.position.Set(0.0f, config.wheel_radius + kGroundY);
  auto wheel = b2_world_.CreateBody(&wheel_body_def);

  b2CircleShape wheel_shape;
  wheel_shape.m_radius = config.wheel_radius;

  b2FixtureDef wheel_fixture_def;
  wheel_fixture_def.shape = &wheel_shape;
  wheel_fixture_def.density = config.wheel_density;
  wheel_fixture_def.friction = config.wheel_friction;
  wheel->CreateFixture(&wheel_fixture_def);

  return wheel;
}

void World::createHinge(b2Body* wheel, b2Body* pole) {
  b2RevoluteJointDef hinge_def;
  hinge_def.bodyA = wheel;
  hinge_def.bodyB = pole;
  hinge_def.localAnchorA.Set(0.0f, 0.0f);
  hinge_def.localAnchorB.Set(0.0f, 0.0f);
  b2_world_.CreateJoint(&hinge_def);
}

World::World(float initial_angle, float target_position, const Unicycle* domain)
    : b2_world_(b2Vec2(0, -domain->config().gravity)),
      target_position_(target_position),
      domain_(domain) {
  createGround(target_position);
  wheel_ = createWheel();
  pole_ = createPole(initial_angle);
  createHinge(wheel_, pole_);
}

bool World::simStep() {
  constexpr float32 kTimeStep = 1.0f / 50.0f;
  constexpr int32 kVelocityIterations = 5;
  constexpr int32 kPositionIterations = 5;

  const auto& config = domain_->config();

  // box2d: simulate one step
  b2_world_.Step(kTimeStep, kVelocityIterations, kPositionIterations);

  // check wheel distance
  const auto distance = wheelDistance();
  if (distance < -config.max_distance || distance > config.max_distance)
    return false;

  // check pole angle
  const auto max_angle = math::degreesToRadians(config.max_angle);
  const auto pole_angle = poleAngle();
  if (pole_angle < -max_angle || pole_angle > max_angle)
    return false;

  return true;
}

void World::turnWheel(float torque) {
  const auto& config = domain_->config();

  // guard against NaNs
  if (isnan(torque)) {
    return;
  }

  // discrete control forces?
  if (config.discrete_controls && torque != 0) {
    const auto magnitude = config.discrete_torque_magnitude;
    torque = torque > 0 ? +magnitude : -magnitude;
  }

  // cap the maximum force magnitude
  // (applies to the discrete inputs as well)
  if (torque < -config.max_torque) {
    torque = -config.max_torque;
  } else if (torque > config.max_torque) {
    torque = config.max_torque;
  }

  wheel_->ApplyTorque(torque, true);
}

}  // namespace unicycle
