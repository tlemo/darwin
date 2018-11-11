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

#include "harvester.h"
#include "robot.h"
#include "world.h"
#include "world_map.h"

#include <core/utils.h>
#include <core/darwin.h>
#include <core/evolution.h>
#include <core/logging.h>
#include <core/parallel_for_each.h>

#include <memory>
#include <vector>
using namespace std;

using core::log;

namespace harvester {

void init() {
  darwin::registry()->domains.add<Factory>("harvester");
}

Harvester::Harvester() {
  inputs_ = Robot::inputsCount();
  outputs_ = Robot::outputsCount();
}

// TODO: try a different approach: for(genotype) { for(map) ... }
bool Harvester::evaluatePopulation(darwin::Population* population) const {
  darwin::StageScope stage("Evaluate population");

  // TODO: move this to darwin?
  const int generation = population->generation();
  log("\n. generation %d\n", generation);

  // generate test maps
  vector<unique_ptr<WorldMap>> test_world_maps(g_config.test_maps);
  pp::for_each(test_world_maps, [&](int, unique_ptr<WorldMap>& test_map) {
    test_map = make_unique<WorldMap>(g_config.map_height, g_config.map_width);
    CHECK(test_map->generate());
  });

  // "grow" robots from each genotype in the population
  vector<Robot> robots(population->size());

  {
    darwin::StageScope stage("Ontogenesis");
    pp::for_each(robots, [&](int index, Robot& robot) {
      auto genotype = population->genotype(index);
      robot.grow(genotype, g_config.initial_health);
      genotype->fitness = 0;
    });
  }

  // evaluate the robots on each test world map
  {
    darwin::StageScope stage("Evaluate test maps", test_world_maps.size());
    for (int map_index = 0; map_index < test_world_maps.size(); ++map_index) {
      log(" ... test world map %d\n", map_index);
      const WorldMap& template_map = *test_world_maps[map_index];

      {
        darwin::StageScope stage("Evaluate one map", robots.size());
        pp::for_each(robots, [&](int robot_index, Robot& robot) {
          World sandbox(template_map, &robot);

          // TODO: revisit (a cleaner pattern?)
          sandbox.simInit();
          while (robot.alive())
            sandbox.simStep();

          population->genotype(robot_index)->fitness +=
              robot.fitness() / test_world_maps.size();

          darwin::ProgressManager::reportProgress();
        });
      }

      darwin::ProgressManager::reportProgress();
    }
  }

  log("\n");
  return false;
}

unique_ptr<darwin::Domain> Factory::create(const core::PropertySet& config) {
  g_config.copyFrom(config);
  return make_unique<Harvester>();
}

unique_ptr<core::PropertySet> Factory::defaultConfig(darwin::ComplexityHint hint) const {
  auto config = make_unique<Config>();
  switch (hint) {
    case darwin::ComplexityHint::Minimal:
      config->test_maps = 2;
      config->initial_health = 500;
      break;

    case darwin::ComplexityHint::Balanced:
      config->test_maps = 5;
      break;

    case darwin::ComplexityHint::Extra:
      config->test_maps = 10;
      config->initial_health = 5000;
      break;
  }
  return config;
}

}  // namespace harvester
