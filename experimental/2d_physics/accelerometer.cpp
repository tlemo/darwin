
#include "accelerometer.h"

#include <core/utils.h>

namespace phys {

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

}  // namespace phys
