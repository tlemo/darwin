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

struct DroneConfig {
  b2Vec2 position{};
  float angle = 0;
  float radius = 0;
  bool camera = false;
  float camera_fov = 90;
  int camera_resolution = 64;
  bool camera_depth = false;
  bool touch_sensor = false;
  int touch_resolution = 16;
  bool accelerometer = false;
  bool compass = false;
  bool lights = false;
  float max_move_force = 5.0f;
  float max_rotate_torque = 1.0f;
  float density = 0.1f;
  float friction = 1.0f;
  float restitution = 0.2f;
  b2Color color;
};

class Drone : public core::NonCopyable {
 public:
  Drone(b2World* world, const DroneConfig& config);

  void postStep(float dt);

  // actuators
  void move(b2Vec2 force);
  void rotate(float torque);

  // sensors
  const Camera* camera() const { return camera_.get(); }
  const sim::TouchSensor* touchSensor() const { return touch_sensor_.get(); }
  const sim::Accelerometer* accelerometer() const { return accelerometer_.get(); }
  const sim::Compass* compass() const { return compass_.get(); }

  b2Body* body() { return body_; }
  const b2Body* body() const { return body_; }

  const auto& config() const { return config_; }

 private:
  b2Body* body_ = nullptr;
  unique_ptr<Camera> camera_;
  unique_ptr<TouchSensor> touch_sensor_;
  unique_ptr<Accelerometer> accelerometer_;
  unique_ptr<Compass> compass_;
  DroneConfig config_;
};

}  // namespace sim
