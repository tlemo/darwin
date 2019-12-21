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

#include <core/sim/camera.h>
#include <core/sim/accelerometer.h>
#include <core/sim/touch_sensor.h>
#include <core/sim/compass.h>
#include <core/utils.h>

#include <third_party/box2d/box2d.h>

namespace sim {

struct CarConfig {
  b2Vec2 position{};
  float angle = 0;
  float length = 2.0f;
  bool camera = false;
  float camera_fov = 90;
  int camera_resolution = 64;
  bool camera_depth = false;
  bool touch_sensor = false;
  int touch_resolution = 16;
  bool accelerometer = false;
  bool compass = false;
  float max_forward_force = 10.0f;
  float max_reverse_force = 4.0f;
  float max_steer_angle = 40.0f;
  float density = 0.01f;
  float tire_traction = 1.5f;
  float friction = 1.0f;
  b2Color color;
};

class Car : public core::NonCopyable {
 public:
  Car(b2World* world, const CarConfig& config);

  void preStep();
  void postStep(float dt);

  //! Accelerates the car on the forward/reverse direction
  //! (positive == forward, negative == reverse)
  void accelerate(float force);

  //! Adjust the steering. `steer_wheeel_position` is the desired
  //! steering wheel position, -1 = max left turn angle, +1 = max right turn angle.
  void steer(float steer_wheel_position);

  //! Apply brakes with the given intensity (0 = no brakes, 1 = maximum braking)
  void brake(float intensity);

  // sensors
  const Camera* camera() const { return camera_.get(); }
  const TouchSensor* touchSensor() const { return touch_sensor_.get(); }
  const Accelerometer* accelerometer() const { return accelerometer_.get(); }
  const Compass* compass() const { return compass_.get(); }

  const b2Vec2& actualVelocity() const { return actual_velocity_; }

  b2Body* body() { return car_body_; }
  const b2Body* body() const { return car_body_; }

  const auto& config() const { return config_; }

 private:
  void createSensors();
  void createWheelFixture(b2Body* body, const b2Vec2& pos);
  void createLightFixture(const b2Vec2& pos, const b2Color& color);
  b2RevoluteJoint* createTurningWheel(b2World* world, const b2Vec2& pos);

  void updateSteering();
  void applyBrakeImpulse(float intensity,
                         const b2Vec2& wheel_normal,
                         const b2Vec2& wheel_center);
  void applyTireLateralImpulse(const b2Vec2& wheel_normal, const b2Vec2& wheel_center);

  float width() const { return config_.length * 0.5f; }
  float frontAxleOffset() const { return config_.length * 0.3f; }
  float rearAxleOffset() const { return config_.length * -0.3f; }

 private:
  b2Body* car_body_ = nullptr;
  b2RevoluteJoint* left_wheel_joint_ = nullptr;
  b2RevoluteJoint* right_wheel_joint_ = nullptr;
  b2Vec2 last_position_ = { 0, 0 };
  b2Vec2 actual_velocity_ = { 0, 0 };
  float target_steer_ = 0;
  float brake_pedal_ = 0;
  unique_ptr<Camera> camera_;
  unique_ptr<TouchSensor> touch_sensor_;
  unique_ptr<Accelerometer> accelerometer_;
  unique_ptr<Compass> compass_;
  const CarConfig config_;
};

}  // namespace sim
