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

#include "classic.h"
#include "feedforward.h"
#include "full_rnn.h"
#include "lstm.h"
#include "lstm_lite.h"
#include "population.h"
#include "rnn.h"

namespace classic {

Config g_config;

size_t g_inputs = 0;
size_t g_outputs = 0;

template <class GENOTYPE>
class Factory : public darwin::PopulationFactory {
  unique_ptr<darwin::Population> create(const core::PropertySet& config,
                                        const darwin::Domain& domain) override {
    g_config.copyFrom(config);
    g_inputs = domain.inputs();
    g_outputs = domain.outputs();
    CHECK(g_inputs > 0);
    CHECK(g_outputs > 0);
    ann::setActivationFunction(g_config.activation_function);
    ann::setGateActivationFunction(g_config.gate_activation_function);
    return make_unique<Population<GENOTYPE>>();
  }

  unique_ptr<core::PropertySet> defaultConfig(
      darwin::ComplexityHint hint) const override {
    auto config = make_unique<Config>();
    switch (hint) {
      case darwin::ComplexityHint::Minimal:
        config->hidden_layers = {};
        break;

      case darwin::ComplexityHint::Balanced:
        config->hidden_layers = { 8 };
        break;

      case darwin::ComplexityHint::Extra:
        config->hidden_layers = { 12, 8, 5 };
        break;
    }
    return config;
  }
};

void init() {
  auto registry = darwin::registry();
  registry->populations.add<Factory<feedforward::Genotype>>("classic.feedforward");
  registry->populations.add<Factory<full_rnn::Genotype>>("classic.full_rnn");
  registry->populations.add<Factory<lstm::Genotype>>("classic.lstm");
  registry->populations.add<Factory<lstm_lite::Genotype>>("classic.lstm_lite");
  registry->populations.add<Factory<rnn::Genotype>>("classic.rnn");
}

}  // namespace classic
