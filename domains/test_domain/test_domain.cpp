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

#include "test_domain.h"
#include "agent.h"

#include <core/evolution.h>
#include <core/exception.h>
#include <core/logging.h>
#include <core/parallel_for_each.h>

namespace test_domain {

TestDomain::TestDomain(const core::PropertySet& config) {
  config_.copyFrom(config);

  // validate configuration
  if (config_.inputs <= 0)
    throw core::Exception("Invalid configuration: inputs must be > 0");
  if (config_.outputs <= 0)
    throw core::Exception("Invalid configuration: outputs must be > 0");
  if (config_.input_range < 0)
    throw core::Exception("Invalid configuration: input_range < 0");
  if (config_.output_range < 0)
    throw core::Exception("Invalid configuration: output_range < 0");
  if (config_.fitness_stddev < 0)
    throw core::Exception("Invalid configuration: fitness_stddev < 0");
  if (config_.fitness_resolution <= 0)
    throw core::Exception("Invalid configuration: fitness_resolution <= 0");
  if (config_.eval_steps <= 0)
    throw core::Exception("Invalid configuration: eval_steps <= 0");
}

size_t TestDomain::inputs() const {
  return config_.inputs;
}

size_t TestDomain::outputs() const {
  return config_.outputs;
}

bool TestDomain::evaluatePopulation(darwin::Population* population) const {
  darwin::StageScope stage("Evaluate population", population->size());

  const int generation = population->generation();
  core::log("\n. generation %d\n", generation);

  pp::for_each(*population, [&](int, darwin::Genotype* genotype) {
    Agent agent(genotype, this);
    genotype->fitness = agent.evaluate();
    darwin::ProgressManager::reportProgress();
  });

  core::log("\n");
  return false;
}

unique_ptr<darwin::Domain> Factory::create(const core::PropertySet& config) {
  return make_unique<TestDomain>(config);
}

unique_ptr<core::PropertySet> Factory::defaultConfig(darwin::ComplexityHint hint) const {
  auto config = make_unique<Config>();
  switch (hint) {
    case darwin::ComplexityHint::Minimal:
    case darwin::ComplexityHint::Balanced:
      break;
    case darwin::ComplexityHint::Extra:
      config->inputs = 256;
      config->outputs = 127;
      config->eval_steps = 1000;
      break;
  }
  return config;
}

}  // namespace test_domain
