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

namespace car_track {

//! Car Track domain configuration
struct Config : public core::PropertySet {
  PROPERTY(drone_radius, float, 0.3f, "Drone size");
  PROPERTY(max_move_force, float, 10.0f, "Maximum force used to move the drone");
  PROPERTY(max_rotate_torque, float, 0.5f, "Maximum torque used to rotate the drone");
  PROPERTY(drone_friction, float, 0.1f, "Drone friction");

  PROPERTY(camera_fov, float, 60, "Camera field of view (FOV)");
  PROPERTY(camera_resolution, int, 64, "Camera resolution");
  PROPERTY(camera_depth, bool, false, "Use camera depth channel");
  
  PROPERTY(touch_sensor, bool, false, "Use the drone's touch sensor");
  PROPERTY(touch_resolution, int, 8, "Touch sensor resolution");
  PROPERTY(accelerometer, bool, false, "Use the drone's accelerometer");
  PROPERTY(compass, bool, false, "Use the drone's compass");

  PROPERTY(track_width, float, 1.8f, "Track width");
  PROPERTY(track_complexity, int, 10, "The approximate number of turns");
  PROPERTY(track_resolution, int, 500, "Number of track segments");

  PROPERTY(test_worlds, int, 3, "Number of test worlds per generation");
  PROPERTY(max_steps, int, 1000, "Maximum number of steps per episode");
};

//! Domain: Car Track
//!
//! Race around a procedurally generated track, using the car's sensors (camera, ...)
//!
//! ![](images/car_track_sandbox.png)
//!
//! ### Inputs
//!
//! 1. Camera: the color (and optionally depth) channels from the drone's camera
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
class CarTrack : public darwin::Domain {
 public:
  explicit CarTrack(const core::PropertySet& config);

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
  darwin::registry()->domains.add<Factory>("car_track");
}

}  // namespace car_track
