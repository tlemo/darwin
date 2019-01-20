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

#include "neat.h"
#include "population.h"

#include <core/darwin.h>

namespace neat {

Config g_config;

int g_inputs = 0;
int g_outputs = 0;

class Factory : public darwin::PopulationFactory {
  unique_ptr<darwin::Population> create(const core::PropertySet& config,
                                        const darwin::Domain& domain) override {
    g_config.copyFrom(config);
    g_inputs = int(domain.inputs());
    g_outputs = int(domain.outputs());
    CHECK(g_inputs > 0);
    CHECK(g_outputs > 0);
    ann::setActivationFunction(g_config.activation_function);
    ann::setGateActivationFunction(g_config.gate_activation_function);
    return make_unique<Population>();
  }

  unique_ptr<core::PropertySet> defaultConfig(
      darwin::ComplexityHint hint) const override {
    auto config = make_unique<Config>();
    switch (hint) {
      case darwin::ComplexityHint::Minimal:
        config->use_lstm_nodes = false;
        config->recurrent_output_nodes = false;
        config->recurrent_hidden_nodes = false;
        config->larva_age = 1;
        config->old_age = 3;
        config->min_species_size = 5;
        break;

      case darwin::ComplexityHint::Balanced:
        config->use_lstm_nodes = false;
        break;

      case darwin::ComplexityHint::Extra:
        config->use_lstm_nodes = true;
        config->recurrent_output_nodes = true;
        config->recurrent_hidden_nodes = true;
        break;
    }
    return config;
  }
};

void init() {
  darwin::registry()->populations.add<Factory>("neat");
}

}  // namespace neat
