
#pragma once

#include <third_party/box2d/box2d.h>

namespace phys {

class Accelerometer {
 public:
  Accelerometer(b2Body* body);

  b2Body* body() const { return body_; }

  // update state (must be called on every simulation step)
  //
  // dt == 0 means instantaneous velocity change (no acceleration)
  //
  void update(float dt);

  // accelerometer measurements
  const b2Vec2& linearAcceleration() const { return linear_acceleration_; }
  float angularAcceleration() const { return angular_acceleration_; }

 private:
  b2Body* body_ = nullptr;
  b2Vec2 linear_acceleration_;
  float angular_acceleration_;
  b2Vec2 last_linear_velocity_;
  float last_angular_velocity_;
};

}  // namespace phys
