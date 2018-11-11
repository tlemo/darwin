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

#include "test_environment.h"

#include <core/utils.h>
#include <core/darwin.h>
#include <core/evolution.h>
#include <core/exception.h>
#include <core/scope_guard.h>
#include <core/universe.h>

#include <third_party/gtest/gtest.h>

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <tuple>
using namespace std;

namespace darwin_smoke_tests {

struct ExperimentConfig {
  string domain_name;
  string population_name;
  int population_size = 0;
  int max_generations = 0;

  friend ostream& operator<<(ostream& stream, const ExperimentConfig& config) {
    return stream << "{ " << config.domain_name << "/" << config.population_name
                  << " population_size=" << config.population_size
                  << " generations=" << config.max_generations << " }";
  }
};

struct SmokeTest : public testing::TestWithParam<ExperimentConfig> {
  SmokeTest() {
    universe = darwin::Universe::open(DarwinTestEnvironment::universePath());
  }

  void runEvolution(const string& config_name,
                    const darwin::EvolutionConfig& evolution_config) {
    auto evolution = darwin::evolution();
    const auto& experiment_conf = GetParam();

    auto events_subscription = evolution->events.subscribe([&](uint32_t hints) {
      if ((hints & darwin::Evolution::EventFlag::EndGeneration) != 0) {
        auto snapshot = evolution->snapshot();
        if (snapshot.generation >= experiment_conf.max_generations)
          evolution->pause();
      }
    });

    SCOPE_EXIT { evolution->events.unsubscribe(events_subscription); };

    darwin::ExperimentSetup experiment_setup;
    experiment_setup.population_size = experiment_conf.population_size;
    experiment_setup.population_name = experiment_conf.population_name;
    experiment_setup.domain_name = experiment_conf.domain_name;
    experiment_setup.population_hint = darwin::ComplexityHint::Minimal;
    experiment_setup.domain_hint = darwin::ComplexityHint::Minimal;

    auto name = core::format("%s/%s/%s",
                             config_name,
                             experiment_setup.domain_name,
                             experiment_setup.population_name);

    auto experiment =
        make_shared<darwin::Experiment>(name, experiment_setup, nullopt, universe.get());

    evolution->newExperiment(experiment, evolution_config);
    evolution->run();
    evolution->waitForState(darwin::Evolution::State::Running);

    // the event callback will pause the evolution
    // when it reaches the target generation
    evolution->waitForState(darwin::Evolution::State::Paused);

    evolution->reset();
    evolution->waitForState(darwin::Evolution::State::Initializing);
  }

  unique_ptr<darwin::Universe> universe;
};

TEST_P(SmokeTest, BalancedResults) {
  darwin::EvolutionConfig evolution_config;
  evolution_config.save_champion_genotype = true;
  evolution_config.fitness_information = darwin::FitnessInfoKind::FullCompressed;
  evolution_config.save_genealogy = false;
  evolution_config.profile_information = darwin::ProfileInfoKind::GenerationOnly;
  runEvolution("balanced", evolution_config);
}

TEST_P(SmokeTest, DetailedResults) {
  darwin::EvolutionConfig evolution_config;
  evolution_config.save_champion_genotype = true;
  evolution_config.fitness_information = darwin::FitnessInfoKind::FullRaw;
  evolution_config.save_genealogy = true;
  evolution_config.profile_information = darwin::ProfileInfoKind::AllStages;
  runEvolution("detailed", evolution_config);
}

vector<ExperimentConfig> everyDomainPopulationCombination() {
  auto registry = darwin::registry();
  CHECK(!registry->domains.empty());
  CHECK(!registry->populations.empty());

  vector<ExperimentConfig> configs;

  constexpr int kPopulationSize = 10;
  constexpr int kGenerations = 5;

  for (const auto& domain_it : registry->domains) {
    for (const auto& population_it : registry->populations) {
      configs.push_back(
          { domain_it.first, population_it.first, kPopulationSize, kGenerations });
    }
  }

  return configs;
}

INSTANTIATE_TEST_CASE_P(All,
                        SmokeTest,
                        testing::ValuesIn(everyDomainPopulationCombination()));

}  // namespace darwin_smoke_tests
