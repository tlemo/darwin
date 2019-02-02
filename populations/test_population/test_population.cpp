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

#include "test_population.h"
#include "population.h"

#include <core/darwin.h>

namespace test_population {

class Factory : public darwin::PopulationFactory {
  unique_ptr<darwin::Population> create(const core::PropertySet& config,
                                        const darwin::Domain& domain) override {
    return make_unique<Population>(config, domain);
  }

  unique_ptr<core::PropertySet> defaultConfig(
      darwin::ComplexityHint hint) const override {
    auto config = make_unique<Config>();
    switch (hint) {
      case darwin::ComplexityHint::Minimal:
        config->input_range = 10.0f;
        config->output_range = 100.0f;
        config->random_outputs = true;
        break;
      case darwin::ComplexityHint::Balanced:
        break;
      case darwin::ComplexityHint::Extra:
        break;
    }
    return config;
  }
};

void init() {
  darwin::registry()->populations.add<Factory>("test_population");
}

}  // namespace test_population
