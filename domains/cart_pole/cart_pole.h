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

#include <core/darwin.h>
#include <core/evolution.h>
#include <core/logging.h>

namespace cart_pole {

//! Cart-Pole domain configuration
struct Config : public core::PropertySet {};

//! Domain: Cart-Pole
class CartPole : public darwin::Domain {
 public:
  explicit CartPole(const core::PropertySet& config);

  size_t inputs() const override { return 1; }
  size_t outputs() const override { return 1; }

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
    return config;
  }
};

inline void init() {
  darwin::registry()->domains.add<Factory>("cart_pole");
}

}  // namespace cart_pole
