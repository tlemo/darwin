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

#include "unicycle.h"
#include "agent.h"
#include "world.h"

#include <core/evolution.h>
#include <core/exception.h>
#include <core/logging.h>
#include <core/parallel_for_each.h>

#include <random>
using namespace std;

namespace unicycle {

Unicycle::Unicycle(const core::PropertySet& config) {
  config_.copyFrom(config);
  validateConfiguration();
}

size_t Unicycle::inputs() const {
  return Agent::inputs(config_);
}

size_t Unicycle::outputs() const {
  return Agent::outputs(config_);
}

bool Unicycle::evaluatePopulation(darwin::Population* population) const {
  darwin::StageScope stage("Evaluate population");

  const int generation = population->generation();
  core::log("\n. generation %d\n", generation);

  // reset the fitness values
  pp::for_each(*population,
               [&](int, darwin::Genotype* genotype) { genotype->fitness = 0; });

  // evaluate each genotype (over N worlds)
  for (int world_index = 0; world_index < config_.test_worlds; ++world_index) {
    darwin::StageScope stage("Evaluate one world", population->size());
    core::log(" ... world %d\n", world_index);

    const float initial_angle_1 = randomInitialAngle();
    const float initial_angle_2 = randomInitialAngle();

    pp::for_each(*population, [&](int, darwin::Genotype* genotype) {
      World world(initial_angle_1, initial_angle_2, this);
      Agent agent(genotype, &world);

      // simulation loop
      int step = 0;
      for (; step < config_.max_steps; ++step) {
        agent.simStep();
        if (!world.simStep())
          break;
      }
      CHECK(step > 0);

      // the fitness is the average number of steps over all test worlds
      genotype->fitness += float(step) / config_.test_worlds;

      darwin::ProgressManager::reportProgress();
    });
  }

  core::log("\n");
  return false;
}

float Unicycle::randomInitialAngle() const {
  random_device rd;
  default_random_engine rnd(rd());
  uniform_real_distribution<float> dist(-config_.max_initial_angle,
                                        config_.max_initial_angle);
  return dist(rnd);
}

// validate the configuration
// (just a few obvious sanity checks for values which would completly break the domain,
// nonsensical configurations are still possible)
void Unicycle::validateConfiguration() {
  if (config_.max_distance <= 0)
    throw core::Exception("Invalid configuration: max_distance <= 0");
  if (config_.max_angle >= 90)
    throw core::Exception("Invalid configuration: max_angle >= 90");
  if (config_.max_initial_angle >= config_.max_angle)
    throw core::Exception("Invalid configuration: max_initial_angle >= max_angle");
  if (config_.pole_1_length <= 0)
    throw core::Exception("Invalid configuration: pole_1_length must be positive");
  if (config_.pole_1_density <= 0)
    throw core::Exception("Invalid configuration: pole_1_density must be positive");
  if (config_.pole_2_length <= 0)
    throw core::Exception("Invalid configuration: pole_2_length must be positive");
  if (config_.pole_2_density <= 0)
    throw core::Exception("Invalid configuration: pole_2_density must be positive");
  if (config_.cart_density < 0)
    throw core::Exception("Invalid configuration: cart_density must be positive or 0");

  if (inputs() < 1)
    throw core::Exception("Invalid configuration: at least one input must be selected");

  if (config_.test_worlds < 1)
    throw core::Exception("Invalid configuration: test_worlds < 1");
  if (config_.max_steps < 1)
    throw core::Exception("Invalid configuration: max_steps < 1");
}

unique_ptr<darwin::Domain> Factory::create(const core::PropertySet& config) {
  return make_unique<Unicycle>(config);
}

unique_ptr<core::PropertySet> Factory::defaultConfig(darwin::ComplexityHint hint) const {
  auto config = make_unique<Config>();
  switch (hint) {
    case darwin::ComplexityHint::Minimal:
      config->test_worlds = 2;
      config->max_steps = 100;
      break;

    case darwin::ComplexityHint::Balanced:
      break;

    case darwin::ComplexityHint::Extra:
      config->test_worlds = 10;
      config->max_steps = 10000;
      break;
  }
  return config;
}

}  // namespace unicycle
