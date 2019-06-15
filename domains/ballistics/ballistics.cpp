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

#include "ballistics.h"
#include "agent.h"
#include "world.h"

#include <core/evolution.h>
#include <core/exception.h>
#include <core/logging.h>
#include <core/parallel_for_each.h>

#include <random>
using namespace std;

namespace ballistics {

Ballistics::Ballistics(const core::PropertySet& config) {
  config_.copyFrom(config);
  validateConfiguration();
}

size_t Ballistics::inputs() const {
  return Agent::kInputs;
}

size_t Ballistics::outputs() const {
  return Agent::kOutputs;
}

bool Ballistics::evaluatePopulation(darwin::Population* population) const {
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

    const b2Vec2 target_position = randomTargetPosition();
    const float target_distance = target_position.Length();

    pp::for_each(*population, [&](int, darwin::Genotype* genotype) {
      World world(target_position, this);

      Agent agent(genotype);
      world.fireProjectile(agent.aim(target_position.x, target_position.y));

      // simulation loop
      float closest_distance = target_distance;
      do {
        const float distance = (world.projectilePosition() - target_position).Length();
        if (distance < closest_distance) {
          closest_distance = distance;
        }
      } while (world.simStep());

      float fitness = 1.0f - closest_distance / target_distance;
      if (closest_distance <= config_.target_radius + config_.projectile_radius) {
        fitness += config_.target_hit_bonus;
      }

      genotype->fitness += fitness / config_.test_worlds;

      darwin::ProgressManager::reportProgress();
    });
  }

  core::log("\n");
  return false;
}

b2Vec2 Ballistics::randomTargetPosition() const {
  random_device rd;
  default_random_engine rnd(rd());
  uniform_real_distribution<float> dist_x(config_.range_min_x, config_.range_max_x);
  uniform_real_distribution<float> dist_y(config_.range_min_y, config_.range_max_y);
  return b2Vec2(dist_x(rnd), dist_y(rnd));
}

// validate the configuration
// (just a few obvious sanity checks for values which would completly break the domain,
// nonsensical configurations are still possible)
void Ballistics::validateConfiguration() {
  if (config_.gravity < 0)
    throw core::Exception("Invalid configuration: gravity < 0");

  if (config_.range_min_x > config_.range_max_x)
    throw core::Exception("Invalid configuration: range_min_x > range_max_x");
  if (config_.range_min_y > config_.range_max_y)
    throw core::Exception("Invalid configuration: range_min_y > range_max_y");

  if (config_.target_radius <= 0)
    throw core::Exception("Invalid configuration: target_radius <= 0");

  if (config_.target_hit_bonus < 0)
    throw core::Exception("Invalid configuration: target_hit_bonus < 0");
  if (config_.target_hit_bonus > 1)
    throw core::Exception("Invalid configuration: target_hit_bonus > 1");

  if (config_.projectile_radius <= 0)
    throw core::Exception("Invalid configuration: projectile_radius <= 0");
  if (config_.projectile_velocity <= 0)
    throw core::Exception("Invalid configuration: projectile_velocity <= 0");

  if (config_.test_worlds < 1)
    throw core::Exception("Invalid configuration: test_worlds < 1");
}

unique_ptr<darwin::Domain> Factory::create(const core::PropertySet& config) {
  return make_unique<Ballistics>(config);
}

unique_ptr<core::PropertySet> Factory::defaultConfig(darwin::ComplexityHint hint) const {
  auto config = make_unique<Config>();
  switch (hint) {
    case darwin::ComplexityHint::Minimal:
      config->test_worlds = 2;
      break;

    case darwin::ComplexityHint::Balanced:
      break;

    case darwin::ComplexityHint::Extra:
      config->test_worlds = 10;
      break;
  }
  return config;
}

}  // namespace ballistics
