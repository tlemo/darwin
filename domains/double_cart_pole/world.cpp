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

namespace double_cart_pole {

World::World(float initial_angle, const CartPole* domain)
    : b2_world_(b2Vec2(0, -domain->config().gravity)), domain_(domain) {
  const auto& config = domain_->config();

  // ground
  constexpr float kGroundY = 0.1f;
  b2EdgeShape ground_shape;
  ground_shape.Set(b2Vec2(-config.max_distance, 0), b2Vec2(config.max_distance, 0));

  b2BodyDef ground_body_def;
  ground_body_def.position.Set(0, kGroundY);
  auto ground = b2_world_.CreateBody(&ground_body_def);
  ground->CreateFixture(&ground_shape, 0.0f);

  // cart
  constexpr float kCartWidth = 0.2f;
  constexpr float kCartHeight = 0.05f;
  b2PolygonShape cart_shape;
  cart_shape.SetAsBox(kCartWidth, kCartHeight);

  b2BodyDef cart_body_def;
  cart_body_def.type = b2_dynamicBody;
  cart_body_def.position.Set(0.0f, kCartHeight + kGroundY);
  cart_ = b2_world_.CreateBody(&cart_body_def);

  b2FixtureDef cart_fixture_def;
  cart_fixture_def.shape = &cart_shape;
  cart_fixture_def.density = config.cart_density;
  cart_fixture_def.friction = config.cart_friction;
  cart_->CreateFixture(&cart_fixture_def);

  // pole
  constexpr float kPoleHalfWidth = 0.02f;
  const float kPoleHalfHeight = config.pole_length / 2;
  b2PolygonShape pole_shape;
  pole_shape.SetAsBox(kPoleHalfWidth, kPoleHalfHeight, b2Vec2(0, kPoleHalfHeight), 0.0f);

  b2BodyDef pole_body_def;
  pole_body_def.type = b2_dynamicBody;
  pole_body_def.position.Set(0.0f, kCartHeight + kGroundY);
  pole_body_def.angle = math::degreesToRadians(initial_angle);
  pole_ = b2_world_.CreateBody(&pole_body_def);

  b2FixtureDef pole_fixture_def;
  pole_fixture_def.shape = &pole_shape;
  pole_fixture_def.density = config.pole_density;
  pole_->CreateFixture(&pole_fixture_def);

  // hinge
  b2RevoluteJointDef hinge_def;
  hinge_def.bodyA = cart_;
  hinge_def.bodyB = pole_;
  hinge_def.localAnchorA.Set(0.0f, 0.0f);
  hinge_def.localAnchorB.Set(0.0f, 0.0f);
  b2_world_.CreateJoint(&hinge_def);
}

bool World::simStep() {
  constexpr float32 kTimeStep = 1.0f / 50.0f;
  constexpr int32 kVelocityIterations = 5;
  constexpr int32 kPositionIterations = 5;

  const auto& config = domain_->config();

  // box2d: simulate one step
  b2_world_.Step(kTimeStep, kVelocityIterations, kPositionIterations);

  // check cart distance
  const auto distance = cartDistance();
  if (distance < -config.max_distance || distance > config.max_distance)
    return false;

  // check pole angle
  const auto pole_angle = poleAngle();
  const auto max_angle = math::degreesToRadians(config.max_angle);
  if (pole_angle < -max_angle || pole_angle > max_angle)
    return false;

  return true;
}

void World::moveCart(float force) {
  const auto& config = domain_->config();
  
  // guard against NaNs
  if (isnan(force)) {
    return;
  }

  // discrete control forces?
  if (config.discrete_controls && force != 0) {
    const auto magnitude = config.discrete_force_magnitude;
    force = force > 0 ? +magnitude : -magnitude;
  }
  
  // cap the maximum force magnitude
  // (applies to the discrete inputs as well)
  if (force < -config.max_force) {
    force = -config.max_force;
  } else if (force > config.max_force) {
    force = config.max_force;
  }
  
  cart_->ApplyForceToCenter(b2Vec2(force, 0), true);
}

}  // namespace double_cart_pole
