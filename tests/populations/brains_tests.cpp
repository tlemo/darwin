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

#include "dummy_domain.h"

#include <core/utils.h>
#include <core/darwin.h>
#include <core/parallel_for_each.h>

#include <populations/cgp/cgp.h>
#include <populations/classic/classic.h>
#include <populations/test_population/test_population.h>
#include <populations/neat/neat.h>

#include <third_party/gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>
using namespace std;

namespace brains_tests {

struct BrainsTest : public testing::TestWithParam<string> {
  static constexpr int kPopulationSize = 100;

  void initialize(size_t inputs, size_t outputs) {
    const auto& population_name = GetParam();

    auto factory = darwin::registry()->populations.find(population_name);
    CHECK(factory != nullptr);

    domain = make_unique<DummyDomain>(inputs, outputs);

    auto config = factory->defaultConfig(darwin::ComplexityHint::Extra);

    if (auto neat_config = dynamic_cast<neat::Config*>(config.get())) {
      updateConfig(neat_config);
    } else if (auto classic_config = dynamic_cast<classic::Config*>(config.get())) {
      updateConfig(classic_config);
    } else if (auto cgp_config = dynamic_cast<cgp::Config*>(config.get())) {
      updateConfig(cgp_config);
    } else if (auto test_config = dynamic_cast<test_population::Config*>(config.get())) {
      updateConfig(test_config);
    } else {
      FATAL("Unexpected population type");
    }

    population = factory->create(*config, *domain);
    CHECK(population);

    population->createPrimordialGeneration(kPopulationSize);
  }

  void updateConfig(test_population::Config* config) {
    config->input_range = 5.0f;
    config->output_range = 10.0f;
    config->random_outputs = true;
  }

  void updateConfig(cgp::Config* config) {
    config->rows = 32;
    config->columns = 17;
    config->levels_back = 8;
    config->outputs_use_levels_back = false;
    config->fn_basic_constants = true;
    config->fn_transcendental_constants = true;
    config->fn_basic_arithmetic = true;
    config->fn_extra_arithmetic = true;
    config->fn_common_math = true;
    config->fn_extra_math = true;
    config->fn_trigonometric = true;
    config->fn_hyperbolic = true;
    config->fn_ann_activation = true;
    config->fn_comparisons = true;
    config->fn_logic_gates = true;
    config->fn_conditional = true;
  }

  void updateConfig(neat::Config* config) {
    config->use_lstm_nodes = true;
    config->normalize_input = true;
    config->normalize_output = true;
  }

  void updateConfig(classic::Config* config) {
    const size_t inputs = domain->inputs();
    const size_t outputs = domain->outputs();
    config->hidden_layers.push_back(inputs);
    config->hidden_layers.push_back(inputs + outputs);
    config->hidden_layers.push_back(outputs);
    config->normalize_input = true;
    config->normalize_output = true;
  }

  void smokeTest() {
    vector<unique_ptr<darwin::Brain>> brains(population->size());
    pp::for_each(brains, [&](int index, unique_ptr<darwin::Brain>& brain) {
      auto genotype = population->genotype(index);
      brain = genotype->grow();

      // set inputs
      for (size_t i = 0; i < domain->inputs(); ++i) {
        const float value = i % 2 ? 1.0f : -1.0f;
        brain->setInput(int(i), value);
      }

      brain->think();

      // consume outputs
      for (size_t i = 0; i < domain->outputs(); ++i) {
        [[maybe_unused]] float value = brain->output(int(i));
      }
    });
  }

  unique_ptr<DummyDomain> domain;
  unique_ptr<darwin::Population> population;
};

TEST_P(BrainsTest, MinSmokeTest) {
  constexpr int kInputs = 1;
  constexpr int kOutputs = 1;
  initialize(kInputs, kOutputs);
  smokeTest();
}

TEST_P(BrainsTest, MediumSmokeTest) {
  constexpr int kInputs = 5;
  constexpr int kOutputs = 3;
  initialize(kInputs, kOutputs);
  smokeTest();
}

TEST_P(BrainsTest, LargeSmokeTest) {
  constexpr int kInputs = 30;
  constexpr int kOutputs = 10;
  initialize(kInputs, kOutputs);
  smokeTest();
}

vector<string> everyPopulation() {
  auto registry = darwin::registry();
  CHECK(!registry->populations.empty());

  vector<string> populations;
  for (const auto& population_it : registry->populations) {
    populations.push_back(population_it.first);
  }

  return populations;
}

INSTANTIATE_TEST_CASE_P(All, BrainsTest, testing::ValuesIn(everyPopulation()));

}  // namespace brains_tests
