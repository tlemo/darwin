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

#include "agent.h"

#include <core/random.h>

#include <cmath>
#include <random>
using namespace std;

namespace test_domain {

Agent::Agent(const darwin::Genotype* genotype, const TestDomain* domain)
    : brain_(genotype->grow()), domain_(domain) {
  CHECK(domain_ != nullptr);
}

float Agent::evaluate() {
  const auto& config = domain_->config();

  default_random_engine rnd(core::randomSeed());
  uniform_real_distribution<float> dist_input(-config.input_range, +config.input_range);

  // "evaluation" steps
  for (int step = 0; step < config.eval_steps; ++step) {
    // inputs
    for (int i = 0; i < config.inputs; ++i) {
      brain_->setInput(i, dist_input(rnd));
    }

    brain_->think();

    // validate output values
    for (int i = 0; i < config.outputs; ++i) {
      const float output_value = brain_->output(i);
      CHECK(!isnan(output_value));
      CHECK(output_value >= -config.output_range);
      CHECK(output_value <= +config.output_range);
    }
  }

  // synthetic fitness value
  normal_distribution<float> dist_fitness(config.fitness_mean, config.fitness_stddev);
  const float fitness_value = dist_fitness(rnd);
  return int(fitness_value / config.fitness_resolution) * config.fitness_resolution;
}

}  // namespace test_domain
