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

#pragma once

#include "double_cart_pole.h"

#include <third_party/box2d/box2d.h>

namespace double_cart_pole {

class World {
  static constexpr float kCartHalfWidth = 0.2f;
  static constexpr float kCartHalfHeight = 0.05f;
  static constexpr float kPoleHalfWidth = 0.02f;
  static constexpr float kGroundY = 0.1f;

 public:
  World(float initial_angle_1, float initial_angle_2, const DoubleCartPole* domain);
  
  // advances the physical simulation one step, returning false
  // if the state reaches one of the termination conditions
  bool simStep();

  // world features
  float cartDistance() const { return cart_->GetPosition().x; }
  float cartVelocity() const { return cart_->GetLinearVelocity().x; }
  float pole1Angle() const { return pole_1_->GetAngle(); }
  float pole1AngularVelocity() const { return pole_1_->GetAngularVelocity(); }
  float pole2Angle() const { return pole_2_->GetAngle(); }
  float pole2AngularVelocity() const { return pole_2_->GetAngularVelocity(); }
  
  // actuators
  void moveCart(float force);
  
  const DoubleCartPole* domain() const { return domain_; }

  b2World* box2dWorld() { return &b2_world_; }
  
 private:
  b2Body* createPole(float length, float density, float initial_angle);
  b2Body* createCart(float density, float friction);
  void createHinge(b2Body* cart, b2Body* pole);
  
 private:
  b2World b2_world_;

  b2Body* cart_ = nullptr;
  b2Body* pole_1_ = nullptr;
  b2Body* pole_2_ = nullptr;
  
  const DoubleCartPole* domain_ = nullptr;
};

}  // namespace double_cart_pole
