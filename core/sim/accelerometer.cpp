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

#include "accelerometer.h"

#include <core/utils.h>

namespace sim {

Accelerometer::Accelerometer(b2Body* body) : body_(body) {
  linear_acceleration_.SetZero();
  angular_acceleration_ = 0;
  last_linear_velocity_.SetZero();
  last_angular_velocity_ = 0;
}

void Accelerometer::update(float dt) {
  CHECK(dt >= 0);

  const auto linear_velocity = body_->GetLinearVelocity();
  const auto angular_velocity = body_->GetAngularVelocity();

  if (dt == 0) {
    linear_acceleration_.SetZero();
    angular_acceleration_ = 0;
  } else {
    const float inv_dt = 1.0f / dt;

    // linear acceleration
    linear_acceleration_ =
        body_->GetLocalVector(linear_velocity - last_linear_velocity_) * inv_dt;
    const float scale = fmaxf(kMaxLinearAcceleration, linear_acceleration_.Length());
    linear_acceleration_ *= 1 / scale;

    // angular acceleration
    angular_acceleration_ = (angular_velocity - last_angular_velocity_) * inv_dt;
    angular_acceleration_ /= kMaxAngularAcceleration;
    angular_acceleration_ = fmaxf(angular_acceleration_, -1);
    angular_acceleration_ = fminf(angular_acceleration_, 1);
  }

  last_linear_velocity_ = linear_velocity;
  last_angular_velocity_ = angular_velocity;
}

}  // namespace sim
