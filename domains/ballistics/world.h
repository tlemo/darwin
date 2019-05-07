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

#include "ballistics.h"

#include <third_party/box2d/box2d.h>

namespace ballistics {

class World {
 public:
  static constexpr float kPoleHalfWidth = 0.02f;
  static constexpr float kGroundY = 0.1f;
  static constexpr float kGroundFriction = 100.0f;

 public:
  World(float initial_angle, float target_position, const Ballistics* domain);

  // advances the physical simulation one step, returning false
  // if the state reaches one of the termination conditions
  bool simStep();

  // world features
  float wheelDistance() const { return wheel_->GetPosition().x; }
  float wheelVelocity() const { return wheel_->GetLinearVelocity().x; }
  float poleAngle() const { return pole_->GetAngle(); }
  float poleAngularVelocity() const { return pole_->GetAngularVelocity(); }

  float targetPosition() const { return target_position_; }
  void setTargetPosition(float target_position);

  // actuators
  void turnWheel(float torque);
  
  const Ballistics* domain() const { return domain_; }

  // extra reward for staying close to the target position
  float fitnessBonus() const { return fitness_bonus_; }

  b2World* box2dWorld() { return &b2_world_; }
  
 private:
  b2Body* createGround();
  b2Body* createPole(float initial_angle);
  b2Body* createWheel();
  void createHinge(b2Body* wheel, b2Body* pole);
  
 private:
  b2World b2_world_;
  b2Body* wheel_ = nullptr;
  b2Body* pole_ = nullptr;
  
  float fitness_bonus_ = 0;
  float target_position_ = 0;
  const Ballistics* domain_ = nullptr;
};

}  // namespace ballistics
