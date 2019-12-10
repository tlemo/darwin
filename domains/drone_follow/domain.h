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

#include <core/darwin.h>
#include <core/properties.h>
#include <core/sim/drone.h>

#include <third_party/box2d/box2d.h>

namespace drone_follow {

//! Drone Follow domain configuration
struct Config : public core::PropertySet {
  PROPERTY(drone_radius, float, 0.5f, "Drone size");
  PROPERTY(drone_lights, bool, true, "Drone has navigation lights");
  PROPERTY(max_move_force, float, 5.0f, "Maximum force used to move the drone");
  PROPERTY(max_rotate_torque, float, 1.0f, "Maximum torque used to rotate the drone");

  PROPERTY(camera_fov, float, 60, "Camera field of view (FOV)");
  PROPERTY(camera_resolution, int, 64, "Camera resolution");
  PROPERTY(camera_depth, bool, false, "Use camera depth channel");
  
  PROPERTY(touch_sensor, bool, false, "Use the drone's touch sensor");
  PROPERTY(touch_resolution, int, 8, "Touch sensor resolution");
  PROPERTY(accelerometer, bool, false, "Use the drone's accelerometer");
  PROPERTY(compass, bool, false, "Use the drone's compass");

  PROPERTY(target_distance, float, 5.0f, "The ideal following distance");

  PROPERTY(scene_columns, bool, false, "Scene includes a few fixed columns");
  PROPERTY(scene_debris, bool, false, "Scene includes random debris");

  PROPERTY(test_worlds, int, 3, "Number of test worlds per generation");
  PROPERTY(max_steps, int, 1000, "Maximum number of steps per episode");
};

//! Domain: Drone Follow
//!
//! Follow another drone while keeping close to a fixed distance. The target drone is
//! tracking a random path.
//!
//! ![](images/drone_follow_sandbox.png)
//!
//! ### Inputs
//!
//! The inputs are handeled through DroneController
//! 
//! 1. Camera: the color channels from the drone's camera, plus optionally the depth
//!   channel.
//! 2. Touch sensor (optional)
//! 3. Compass (optional)
//! 4. Accelerometer (optional)
//!
//! ### Outputs
//!
//! Output | Value
//! ------:|------
//!    0,1 | force vector (x, y) applied to the drone
//!      2 | torque applied to turn the drone
//!
class DroneFollow : public darwin::Domain {
 public:
  explicit DroneFollow(const core::PropertySet& config);

  size_t inputs() const override;
  size_t outputs() const override;

  bool evaluatePopulation(darwin::Population* population) const override;
  
  const Config& config() const { return config_; }
  const sim::DroneConfig& droneConfig() const { return drone_config_; }
  
 private:
  void validateConfiguration();

 private:
  Config config_;
  sim::DroneConfig drone_config_;
};

class Factory : public darwin::DomainFactory {
  unique_ptr<darwin::Domain> create(const core::PropertySet& config) override;
  unique_ptr<core::PropertySet> defaultConfig(darwin::ComplexityHint hint) const override;
};

inline void init() {
  darwin::registry()->domains.add<Factory>("drone_follow");
}

}  // namespace drone_follow
