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

#include "dummy_domain.h"

#include <core/darwin.h>
#include <core/parallel_for_each.h>
#include <core/utils.h>
#include <core/random.h>

#include <third_party/gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>
#include <random>
using namespace std;

namespace populations_smoke_tests {

struct PopulationsTest : public testing::TestWithParam<string> {
  static constexpr int kPopulationSize = 100;

  void initialize(size_t inputs, size_t outputs) {
    const auto& population_name = GetParam();
    auto factory = darwin::registry()->populations.find(population_name);
    CHECK(factory != nullptr);

    domain = make_unique<DummyDomain>(inputs, outputs);

    auto config = factory->defaultConfig(darwin::ComplexityHint::Extra);
    population = factory->create(*config, *domain);
    CHECK(population);

    population->createPrimordialGeneration(kPopulationSize);
  }

  void validate() {
    // validate the ranking index
    const auto& ranking_index = population->rankingIndex();
    for (size_t i = 1; i < ranking_index.size(); ++i) {
      EXPECT_LE(population->genotype(ranking_index[i])->fitness,
                population->genotype(ranking_index[i - 1])->fitness);
    }

    // create next generation
    population->createNextGeneration();
  }

  unique_ptr<DummyDomain> domain;
  unique_ptr<darwin::Population> population;
};

TEST_P(PopulationsTest, ConstFitnessValues) {
  constexpr int kInputs = 1;
  constexpr int kOutputs = 1;
  initialize(kInputs, kOutputs);

  // "evaluate" the population
  for (size_t i = 0; i < population->size(); ++i) {
    darwin::Genotype* genotype = population->genotype(i);
    genotype->fitness = 0;
  }

  validate();
}

TEST_P(PopulationsTest, RandomFitnessValues) {
  constexpr int kInputs = 1;
  constexpr int kOutputs = 1;
  initialize(kInputs, kOutputs);

  // "evaluate" the population
  default_random_engine rnd(core::randomSeed());
  uniform_int_distribution<int> dist(-3, 3);
  for (size_t i = 0; i < population->size(); ++i) {
    darwin::Genotype* genotype = population->genotype(i);
    genotype->fitness = dist(rnd);
  }

  validate();
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

INSTANTIATE_TEST_CASE_P(All, PopulationsTest, testing::ValuesIn(everyPopulation()));

}  // namespace populations_smoke_tests
