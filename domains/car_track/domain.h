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
#include <core/sim/car.h>

#include <third_party/box2d/box2d.h>

namespace car_track {

//! Car Track domain configuration
struct Config : public core::PropertySet {
  PROPERTY(car_length, float, 1.2f, "Car length");
  PROPERTY(max_forward_force, float, 2.0f, "Max forward move force");
  PROPERTY(max_reverse_force, float, 0.5f, "Max reverse move force");
  PROPERTY(max_steer_angle, float, 40.0f, "Max steering angle");
  PROPERTY(tire_traction, float, 1.5f, "Tire traction (max lateral impulse)");

  PROPERTY(camera_fov, float, 90, "Camera field of view (FOV)");
  PROPERTY(camera_resolution, int, 64, "Camera resolution");
  PROPERTY(camera_depth, bool, false, "Use camera depth channel");

  PROPERTY(touch_sensor, bool, true, "Use the drone's touch sensor");
  PROPERTY(touch_resolution, int, 8, "Touch sensor resolution");
  PROPERTY(accelerometer, bool, true, "Use the drone's accelerometer");
  PROPERTY(compass, bool, true, "Use the drone's compass");

  PROPERTY(track_width, float, 2.5f, "Track width");
  PROPERTY(track_complexity, int, 10, "The approximate number of turns");
  PROPERTY(track_resolution, int, 500, "Number of track segments");
  PROPERTY(curb_width, float, 0.1f, "Curb width");
  PROPERTY(curb_friction, float, 0.5f, "Track's curb friction");
  PROPERTY(track_gates, bool, true, "Generate track gates");
  PROPERTY(solid_gate_posts, bool, true, "Solid gate posts");

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
//!      0 | acceleration force (negative values result in reverse movement)
//!      1 | desired steering angle (-1.0 .. +1.0)
//!
class CarTrack : public darwin::Domain {
 public:
  explicit CarTrack(const core::PropertySet& config);

  size_t inputs() const override;
  size_t outputs() const override;

  bool evaluatePopulation(darwin::Population* population) const override;

  const Config& config() const { return config_; }
  const sim::CarConfig& carConfig() const { return car_config_; }

 private:
  void validateConfiguration();

 private:
  Config config_;
  sim::CarConfig car_config_;
};

class Factory : public darwin::DomainFactory {
  unique_ptr<darwin::Domain> create(const core::PropertySet& config) override;
  unique_ptr<core::PropertySet> defaultConfig(darwin::ComplexityHint hint) const override;
};

inline void init() {
  darwin::registry()->domains.add<Factory>("car_track");
}

}  // namespace car_track
