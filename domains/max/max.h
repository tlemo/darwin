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

#pragma once

#include "robot.h"
#include "world.h"

#include <core/darwin.h>
#include <core/evolution.h>
#include <core/logging.h>
#include <core/parallel_for_each.h>

using core::log;

namespace find_max_value {

//! Domain: Locate the maximum value in an array
//! 
//! This is a minimalist problem intended to evaluate a population's ability to evolve
//! basic memory facilities: locate the maximum value in an array of integers.
//! 
//! This deceptively simple problem requires the emergence of a few capabilities:
//! 
//! - Navigating the one-dimensional array, covering all the values
//! - Remembering values (at least a current maximum)
//! - Comparing integer values (to determine the maximum)
//! 
//! ### Core concepts
//! 
//! - The test "world": a one dimensional array of a random length and initialized with 
//!   random values.
//! 
//! - The agent is a "pointer/index" within the test world.
//! 
//! ### Inputs
//!
//! Input | Value
//! -----:|------
//!     0 | left_antenna (1.0 if the the agent is positioned at the left edge)
//!     1 | right_antenna (1.0 if the the agent is positioned at the right edge)
//!     2 | input_value
//! 
//! ### Outputs
//!
//! Output | Value
//! ------:|------
//!      0 | move_left
//!      1 | move_right
//!      2 | done (Accept current position as the final answer)
//! 
//! The largest output signal determines the agent's action at the current step.
//! 
class FindMaxValue : public darwin::Domain {
 public:
  size_t inputs() const override { return Robot::kInputs; }
  size_t outputs() const override { return Robot::kOutputs; }

  bool evaluatePopulation(darwin::Population* population) const override {
    darwin::StageScope stage("Evaluate population");

    const int generation = population->generation();
    log("\n. generation %d\n", generation);

    // generate test worlds
    vector<World> worlds(g_config.test_worlds);
    pp::for_each(worlds, [&](int, World& world) { world.generate(); });

    // "grow" robots from each genotype in the population
    vector<Robot> robots(population->size());

    {
      darwin::StageScope stage("Ontogenesis");
      pp::for_each(robots, [&](int index, Robot& robot) {
        auto genotype = population->genotype(index);
        robot.grow(genotype);
        genotype->fitness = 0;
      });
    }

    // evaluate the robots in each test world
    {
      darwin::StageScope stage("Evaluate multiple worlds", worlds.size());
      for (int world_index = 0; world_index < worlds.size(); ++world_index) {
        log(" ... world %d\n", world_index);
        const World& world_template = worlds[world_index];

        {
          darwin::StageScope stage("Evaluate one world", robots.size());
          pp::for_each(robots, [&](int robot_index, Robot& robot) {
            World sandbox;
            sandbox.simInit(world_template, &robot);

            while (robot.alive())
              sandbox.simStep();

            population->genotype(robot_index)->fitness += robot.fitness / worlds.size();

            darwin::ProgressManager::reportProgress();
          });
        }

        darwin::ProgressManager::reportProgress();
      }
    }

    log("\n");
    return false;
  }
};

class Factory : public darwin::DomainFactory {
  unique_ptr<darwin::Domain> create(const core::PropertySet& config) override {
    g_config.copyFrom(config);
    return make_unique<FindMaxValue>();
  }

  unique_ptr<core::PropertySet> defaultConfig(
      darwin::ComplexityHint hint) const override {
    auto config = make_unique<Config>();
    switch (hint) {
      case darwin::ComplexityHint::Minimal:
        config->test_worlds = 2;
        config->min_size = kMinSize;
        config->max_size = 10;
        break;

      case darwin::ComplexityHint::Balanced:
        break;

      case darwin::ComplexityHint::Extra:
        config->test_worlds = 30;
        config->min_size = kMinSize;
        config->max_size = 250;
        break;
    }
    return config;
  }
};

inline void init() {
  darwin::registry()->domains.add<Factory>("find_max_value");
}

}  // namespace find_max_value
