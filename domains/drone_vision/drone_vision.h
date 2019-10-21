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

namespace drone_vision {

//! Drone Vision domain configuration
struct Config : public core::PropertySet {
  PROPERTY(drone_radius, float, 0.5f, "Drone size");
  PROPERTY(move_force, float, 5.0f, "The force used to move the drone");
  PROPERTY(rotate_torque, float, 1.0f, "The torque used to rotate the drone");

  PROPERTY(camera_fov, float, 120, "Camera field of view (FOV)");
  PROPERTY(camera_resolution, int, 64, "Camera resolution");
  PROPERTY(camera_depth, bool, false, "Use camera depth channel");

  PROPERTY(test_worlds, int, 5, "Number of test worlds per generation");
  PROPERTY(max_steps, int, 1000, "Maximum number of steps per episode");
};

//! Domain: Drone Vision
//! TODO...
class DroneVision : public darwin::Domain {
 public:
  explicit DroneVision(const core::PropertySet& config);

  size_t inputs() const override;
  size_t outputs() const override;

  bool evaluatePopulation(darwin::Population* population) const override;
  
  const Config& config() const { return config_; }
  
  float randomInitialAngle() const;
  float randomTargetPosition() const;
  
 private:
  void validateConfiguration();

 private:
  Config config_;
};

class Factory : public darwin::DomainFactory {
  unique_ptr<darwin::Domain> create(const core::PropertySet& config) override;
  unique_ptr<core::PropertySet> defaultConfig(darwin::ComplexityHint hint) const override;
};

inline void init() {
  darwin::registry()->domains.add<Factory>("drone_vision");
}

}  // namespace drone_vision
