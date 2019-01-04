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

#include "agent.h"

#include <core/darwin.h>
#include <core/properties.h>

namespace cart_pole {

//! Cart-Pole domain configuration
struct Config : public core::PropertySet {
  PROPERTY(gravity, float, 9.8f, "Gravitational acceleration");

  PROPERTY(max_distance, float, 2.5f, "Maximum distance from the center");
  PROPERTY(max_angle, float, 60.0f, "Maximum angle from vertical");
  PROPERTY(max_initial_angle, float, 10.0f, "Maximum starting angle from vertical");
  PROPERTY(pole_length, float, 1.5f, "Pole length");
  PROPERTY(pole_density, float, 1.0f, "Pole density");
  PROPERTY(cart_density, float, 0.0f, "Cart density");
  PROPERTY(cart_friction, float, 0.0f, "Cart friction");
  PROPERTY(max_force, float, 5.0f, "Maximum force which can be applied to the cart");
  
  PROPERTY(test_worlds, int, 5, "Number of test worlds per generation");
  PROPERTY(max_steps, int, 1000, "Maximum number of steps per episode");

  PROPERTY(discrete_controls,
           bool,
           true,
           "Force the actuator force to fixed +/-discrete_force_magnitude");

  PROPERTY(discrete_force_magnitude,
           float,
           2.5f,
           "The fixed force magnitude used if discrete_controls is true");
};

//! Domain: Cart-Pole
class CartPole : public darwin::Domain {
 public:
  explicit CartPole(const core::PropertySet& config);

  size_t inputs() const override { return Agent::kInputs; }
  size_t outputs() const override { return Agent::kOutputs; }

  bool evaluatePopulation(darwin::Population* population) const override;
  
  const Config& config() const { return config_; }
  
  float randomInitialAngle() const;

 private:
  Config config_;
};

class Factory : public darwin::DomainFactory {
  unique_ptr<darwin::Domain> create(const core::PropertySet& config) override;
  unique_ptr<core::PropertySet> defaultConfig(darwin::ComplexityHint hint) const override;
};

inline void init() {
  darwin::registry()->domains.add<Factory>("cart_pole");
}

}  // namespace cart_pole
