// Copyright 2018 The Darwin Neuroevolution Framework Authors.
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
  
  PROPERTY(test_worlds, int, 10, "Number of test worlds per generation");
  PROPERTY(max_steps, int, 1000, "Maximum number of steps per episode");
};

//! Domain: Cart-Pole
class CartPole : public darwin::Domain {
 public:
  explicit CartPole(const core::PropertySet& config);

  size_t inputs() const override { return Agent::kInputs; }
  size_t outputs() const override { return Agent::kOutputs; }

  bool evaluatePopulation(darwin::Population* population) const override;

 private:
  Config config_;
};

class Factory : public darwin::DomainFactory {
  unique_ptr<darwin::Domain> create(const core::PropertySet& config) override {
    return make_unique<CartPole>(config);
  }

  unique_ptr<core::PropertySet> defaultConfig(
      darwin::ComplexityHint hint) const override {
    auto config = make_unique<Config>();
    switch (hint) {
      case darwin::ComplexityHint::Minimal:
        config->test_worlds = 2;
        config->max_steps = 100;
        break;

      case darwin::ComplexityHint::Balanced:
        break;

      case darwin::ComplexityHint::Extra:
        config->test_worlds = 20;
        config->max_steps = 10000;
        break;
    }
    return config;
  }
};

inline void init() {
  darwin::registry()->domains.add<Factory>("cart_pole");
}

}  // namespace cart_pole
