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

#include "cart_pole.h"
#include "agent.h"
#include "world.h"

#include <core/evolution.h>
#include <core/parallel_for_each.h>
#include <core/logging.h>

namespace cart_pole {

CartPole::CartPole(const core::PropertySet& config) {
  config_.copyFrom(config);
}

bool CartPole::evaluatePopulation(darwin::Population* population) const {
  darwin::StageScope stage("Evaluate population");

  const int generation = population->generation();
  core::log("\n. generation %d\n", generation);

  for (int world_index = 0; world_index < config_.test_worlds; ++world_index) {
    darwin::StageScope stage("Evaluate one world", population->size());
    core::log(" ... world %d\n", world_index);

    // TODO: random start angle
    const float start_angle = 0;
    
    pp::for_each(*population, [&](int, darwin::Genotype* genotype) {
      World world(start_angle, config_);
      Agent agent(genotype, &world);

      // simulation loop
      int step = 0;
      for (; step < config_.max_steps; ++step) {
        agent.simStep();
        if (!world.simStep())
          break;
      }

      genotype->fitness = step;
      darwin::ProgressManager::reportProgress();
    });
  }
  
  core::log("\n");
  return false;
}

}  // namespace cart_pole
