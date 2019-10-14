// Copyright The Darwin Neuroevolution Framework Authors.
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

#include <third_party/box2d/box2d.h>

namespace physics {

//! Basic accelerometer 
//! 
//! \note It treats gravity as a regular force
//! (so it doesn't really measure the "proper acceleration":
//!  https://en.wikipedia.org/wiki/Proper_acceleration)
//! 
class Accelerometer {
  static constexpr float kMaxLinearAcceleration = 100.0f;
  static constexpr float kMaxAngularAcceleration = 120.0f;

 public:
  Accelerometer(b2Body* body);

  b2Body* body() const { return body_; }

  // update state (must be called on every simulation step)
  //
  // dt == 0 means instantaneous velocity change (no acceleration)
  //
  void update(float dt);

  // the linear acceleration vector length is in the [0, 1] range
  const b2Vec2& linearAcceleration() const { return linear_acceleration_; }
  
  // angular acceleration is in the [-1, 1] range
  float angularAcceleration() const { return angular_acceleration_; }

 private:
  b2Body* body_ = nullptr;
  b2Vec2 linear_acceleration_;
  float angular_acceleration_;
  b2Vec2 last_linear_velocity_;
  float last_angular_velocity_;
};

}  // namespace physics
