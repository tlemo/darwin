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

#include <core/utils.h>
#include <core/darwin.h>
#include <core/selection_algorithm.h>
#include <core/roulette_selection.h>
#include <core/cgp_islands_selection.h>
#include <core/truncation_selection.h>
#include <core/random.h>

#include <third_party/gtest/gtest.h>

#include <vector>
#include <memory>
#include <random>
#include <algorithm>
using namespace std;

namespace selection_algorithms_tests {

struct TestGenotypeFactory : public selection::GenotypeFactory {
  int population_size = 0;
  bool valid_genotype = false;

  explicit TestGenotypeFactory(int population_size) : population_size(population_size) {}

  void createPrimordialSeed() override {
    EXPECT_FALSE(valid_genotype);
    valid_genotype = true;
  }

  void replicate(int parent_index) override {
    EXPECT_GE(parent_index, 0);
    EXPECT_LT(parent_index, population_size);
    EXPECT_FALSE(valid_genotype);
    valid_genotype = true;
  }

  void crossover(int parent1, int parent2, float preference) override {
    EXPECT_GE(parent1, 0);
    EXPECT_LT(parent1, population_size);
    EXPECT_GE(parent2, 0);
    EXPECT_LT(parent2, population_size);
    EXPECT_GE(preference, 0.0f);
    EXPECT_LE(preference, 1.0f);
    EXPECT_FALSE(valid_genotype);
    valid_genotype = true;
  }

  void mutate() override { EXPECT_TRUE(valid_genotype); }
};

struct TestGenerationFactory : public selection::GenerationFactory {
  vector<TestGenotypeFactory> genotype_factories;

  explicit TestGenerationFactory(size_t population_size) {
    CHECK(population_size > 0);
    genotype_factories.resize(population_size, TestGenotypeFactory(int(population_size)));
  }

  size_t size() const override { return genotype_factories.size(); }

  TestGenotypeFactory* operator[](size_t index) override {
    EXPECT_LT(index, genotype_factories.size());
    return &genotype_factories[index];
  }

  void postSelectionChecks() const {
    // check that all the genotypes in the new generations are valid
    for (const auto& genotype_factory : genotype_factories) {
      EXPECT_TRUE(genotype_factory.valid_genotype);
    }
  }
};

struct TestGenotype : public darwin::Genotype {
  unique_ptr<darwin::Brain> grow() const override { FATAL("Not implemented"); }
  unique_ptr<darwin::Genotype> clone() const override { FATAL("Not implemented"); }
  json save() const override { FATAL("Not implemented"); }
  void load(const json&) override { FATAL("Not implemented"); }
};

struct TestPopulation : public darwin::Population {
  vector<TestGenotype> genotypes;
  int current_generation = 0;
  selection::SelectionAlgorithm* selection_algorithm = nullptr;

  size_t size() const override { return genotypes.size(); }

  darwin::Genotype* genotype(size_t i) override { return &genotypes[i]; }
  const darwin::Genotype* genotype(size_t i) const override { return &genotypes[i]; }

  int generation() const override { return current_generation; }

  vector<size_t> rankingIndex() const override {
    vector<size_t> ranking_index(genotypes.size());
    for (size_t i = 0; i < ranking_index.size(); ++i) {
      ranking_index[i] = i;
    }
    std::sort(ranking_index.begin(), ranking_index.end(), [&](size_t a, size_t b) {
      return genotypes[a].fitness > genotypes[b].fitness;
    });
    return ranking_index;
  }

  void createPrimordialGeneration(int population_size) override {
    genotypes.clear();
    genotypes.resize(population_size);
    selection_algorithm->newPopulation(this);
  }

  void createNextGeneration() override {
    ++current_generation;
    TestGenerationFactory generation_factory(genotypes.size());
    selection_algorithm->createNextGeneration(&generation_factory);
    generation_factory.postSelectionChecks();
  }

  void setSelectionAlgorithm(selection::SelectionAlgorithm* selection_algorithm) {
    this->selection_algorithm = selection_algorithm;
  }
};

struct SelectionAlgorithmsTest : public testing::TestWithParam<int> {
  TestPopulation population;

  void testAllZeroFitness(int generations_count) {
    for (int generation = 0; generation < generations_count; ++generation) {
      for (size_t i = 0; i < population.size(); ++i) {
        population.genotype(i)->fitness = 0;
      }
      population.createNextGeneration();
    }
  }

  void testRandomFitness(int generations_count) {
    default_random_engine rnd(core::randomSeed());
    uniform_real_distribution<float> dist_fitness(-5, 5);
    for (int generation = 0; generation < generations_count; ++generation) {
      for (size_t i = 0; i < population.size(); ++i) {
        population.genotype(i)->fitness = dist_fitness(rnd);
      }
      population.createNextGeneration();
    }
  }
};

TEST_P(SelectionAlgorithmsTest, RouletteSelection) {
  selection::RouletteSelectionConfig config;
  config.min_fitness = 0.0f;
  config.elite_percentage = 0.3f;
  config.elite_min_fitness = 0.0f;
  config.mutation_only = false;

  selection::RouletteSelection selection_algorithm(config);
  population.setSelectionAlgorithm(&selection_algorithm);
  population.createPrimordialGeneration(GetParam());

  testAllZeroFitness(10);
  testRandomFitness(10);
}

TEST_P(SelectionAlgorithmsTest, RouletteSelection_MutationOnly) {
  selection::RouletteSelectionConfig config;
  config.min_fitness = 0.5f;
  config.elite_percentage = 0.3f;
  config.elite_min_fitness = 0.1f;
  config.mutation_only = true;

  selection::RouletteSelection selection_algorithm(config);
  population.setSelectionAlgorithm(&selection_algorithm);
  population.createPrimordialGeneration(GetParam());

  testAllZeroFitness(10);
  testRandomFitness(10);
}

TEST_P(SelectionAlgorithmsTest, CgpIslandsSelection) {
  selection::CgpIslandsSelectionConfig config;
  config.island_size = 5;
  config.protected_age = 10;
  config.extinction_percentage = 0.6f;

  selection::CgpIslandsSelection selection_algorithm(config);
  population.setSelectionAlgorithm(&selection_algorithm);
  population.createPrimordialGeneration(GetParam());

  testAllZeroFitness(50);
  testRandomFitness(50);
}

TEST_P(SelectionAlgorithmsTest, TruncationSelection) {
  selection::TruncationSelectionConfig config;
  config.elite_percentage = 0.2f;
  config.elite_min_fitness = -1.0f;
  config.elite_mutation_chance = 0.5f;

  selection::TruncationSelection selection_algorithm(config);
  population.setSelectionAlgorithm(&selection_algorithm);
  population.createPrimordialGeneration(GetParam());

  testAllZeroFitness(25);
  testRandomFitness(25);
}

// instantiate the test cases with various population sizes
INSTANTIATE_TEST_CASE_P(All, SelectionAlgorithmsTest, testing::Values(1, 5, 53));

}  // namespace selection_algorithms_tests
