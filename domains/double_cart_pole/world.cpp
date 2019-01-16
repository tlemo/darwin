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

b2Body* World::createPole(float length, float density, float initial_angle) {
  const float kPoleHalfHeight = length / 2;

  b2PolygonShape pole_shape;
  pole_shape.SetAsBox(kPoleHalfWidth, kPoleHalfHeight, b2Vec2(0, kPoleHalfHeight), 0.0f);

  b2BodyDef pole_body_def;
  pole_body_def.type = b2_dynamicBody;
  pole_body_def.position.Set(0.0f, kCartHalfHeight + kGroundY);
  pole_body_def.angle = math::degreesToRadians(initial_angle);
  auto pole = b2_world_.CreateBody(&pole_body_def);

  b2FixtureDef pole_fixture_def;
  pole_fixture_def.shape = &pole_shape;
  pole_fixture_def.density = density;
  pole_fixture_def.filter.groupIndex = -1;
  pole->CreateFixture(&pole_fixture_def);

  return pole;
}

b2Body* World::createCart(float density, float friction) {
  b2PolygonShape cart_shape;
  cart_shape.SetAsBox(kCartHalfWidth, kCartHalfHeight);

  b2BodyDef cart_body_def;
  cart_body_def.type = b2_dynamicBody;
  cart_body_def.position.Set(0.0f, kCartHalfHeight + kGroundY);
  auto cart = b2_world_.CreateBody(&cart_body_def);

  b2FixtureDef cart_fixture_def;
  cart_fixture_def.shape = &cart_shape;
  cart_fixture_def.density = density;
  cart_fixture_def.friction = friction;
  cart->CreateFixture(&cart_fixture_def);

  return cart;
}

void World::createHinge(b2Body* cart, b2Body* pole) {
  b2RevoluteJointDef hinge_def;
  hinge_def.bodyA = cart;
  hinge_def.bodyB = pole;
  hinge_def.localAnchorA.Set(0.0f, 0.0f);
  hinge_def.localAnchorB.Set(0.0f, 0.0f);
  b2_world_.CreateJoint(&hinge_def);
}

World::World(float initial_angle_1, float initial_angle_2, const DoubleCartPole* domain)
    : b2_world_(b2Vec2(0, -domain->config().gravity)), domain_(domain) {
  const auto& config = domain_->config();

  // ground
  b2EdgeShape ground_shape;
  ground_shape.Set(b2Vec2(-config.max_distance, 0), b2Vec2(config.max_distance, 0));

  b2BodyDef ground_body_def;
  ground_body_def.position.Set(0, kGroundY);
  auto ground = b2_world_.CreateBody(&ground_body_def);
  ground->CreateFixture(&ground_shape, 0.0f);

  // cart & poles
  cart_ = createCart(config.cart_density, config.cart_friction);
  pole_1_ = createPole(config.pole_1_length, config.pole_1_density, initial_angle_1);
  pole_2_ = createPole(config.pole_2_length, config.pole_2_density, initial_angle_2);
  createHinge(cart_, pole_1_);
  createHinge(cart_, pole_2_);
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

  const auto max_angle = math::degreesToRadians(config.max_angle);

  // check pole 1 angle
  const auto pole_1_angle = pole1Angle();
  if (pole_1_angle < -max_angle || pole_1_angle > max_angle)
    return false;
    
  // check pole 2 angle
  const auto pole_2_angle = pole2Angle();
  if (pole_2_angle < -max_angle || pole_2_angle > max_angle)
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
