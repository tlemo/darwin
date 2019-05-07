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

namespace ballistics {

//! Ballistics domain configuration
struct Config : public core::PropertySet {
  PROPERTY(gravity, float, 9.8f, "Gravitational acceleration");
  PROPERTY(max_distance, float, 3.0f, "Maximum distance from the center");
  PROPERTY(max_angle, float, 60.0f, "Maximum angle from vertical");
  PROPERTY(max_initial_angle, float, 10.0f, "Maximum starting angle from vertical");
  PROPERTY(pole_length, float, 1.5f, "Pole length");
  PROPERTY(pole_density, float, 1.0f, "Pole density");
  PROPERTY(wheel_radius, float, 0.2f, "Wheel size (radius)");
  PROPERTY(wheel_density, float, 1.0f, "Wheel density");
  PROPERTY(wheel_friction, float, 10.0f, "Wheel friction");
  PROPERTY(max_torque, float, 1.0f, "Maximum torque which can be applied to the wheel");

  PROPERTY(input_pole_angle, bool, true, "Use the pole angle as input");
  PROPERTY(input_angular_velocity, bool, true, "Use the angular velocity as input");
  PROPERTY(input_wheel_distance, bool, true, "Use the wheel distance as input");
  PROPERTY(input_wheel_velocity, bool, true, "Use the wheel linear velocity as input");
  PROPERTY(input_distance_from_target, bool, true, "Distance from target position");

  PROPERTY(test_worlds, int, 5, "Number of test worlds per generation");
  PROPERTY(max_steps, int, 1000, "Maximum number of steps per episode");

  PROPERTY(discrete_controls,
           bool,
           false,
           "Force the actuator force to fixed +/-discrete_force_magnitude");

  PROPERTY(discrete_torque_magnitude,
           float,
           0.5f,
           "The fixed force magnitude used if discrete_controls is true");
};

//! Domain: Ballistics
//!
//! Yet another inverted pendulum variation: this time the pole is attached to a wheel
//! and the agent tries to keep the pole balanced by applying torque to the wheel, similar
//! to riding an ballistics.
//! 
//! ![](images/unicycle_sandbox.png)
//!
//! The ballistics starts in the middle (x = 0) and the initial pole angles is a random 
//! value in the `[-max_initial_angle, +max_initial_angle]` range. The pole is considered
//! balanced if it remains between `-max_angle` and `+max_angle`. The ballistics's wheel 
//! must stay between `-max_distance` and `+max_distance`.
//! 
//! This problem introduces an additional goal: keep the ballistics close to a random target
//! position. This is rewarded by a fitness bonus inversely proportional to the absolute
//! distance from the target (but only if the pole is balanced for the whole episode)
//!
//! \sa cart_pole::CartPole
//! \sa double_cart_pole::DoubleCartPole
//!
//! ### Inputs
//!
//! The inputs are configurable by individually selecting at least one of:
//! - pole_angle (from vertical)
//! - pole angular_velocity
//! - wheel_distance (from the center)
//! - wheel_velocity (horizontal velocity component)
//! - distance_from_target
//!
//! Input | Value
//! -----:|------
//!     0 | pole_angle
//!     1 | angular_velocity
//!     2 | wheel_distance
//!     3 | wheel_velocity
//!     4 | distance_from_target
//!
//! ### Outputs
//!
//! The single output indicates the torque to be applied to the wheel. This can
//! be discrete (fixed +/-discrete_torque_magnitude depending on the sign of the output) 
//! or can be continuous (the output value maps directly to the torque magnitude)
//!
//! Output | Value
//! ------:|------
//!      0 | torque
//!
class Ballistics : public darwin::Domain {
 public:
  explicit Ballistics(const core::PropertySet& config);

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
  darwin::registry()->domains.add<Factory>("ballistics");
}

}  // namespace ballistics
