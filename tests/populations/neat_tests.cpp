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
#include <core/utils.h>
#include <populations/neat/brain.h>
#include <populations/neat/genotype.h>
#include <populations/neat/neat.h>

#include <third_party/json/json.h>
using nlohmann::json;

#include <tests/testcase_output.h>
#include <third_party/gtest/gtest.h>

#include <atomic>
#include <vector>
#include <random>
using namespace std;

namespace neat_tests {

struct NeatTest : public testing::Test {
  static constexpr int kInputs = 2;
  static constexpr int kOutputs = 3;

  NeatTest() {
    auto factory = darwin::registry()->populations.find("neat");
    CHECK(factory != nullptr);

    domain = make_unique<DummyDomain>(kInputs, kOutputs);
    
    config = make_unique<neat::Config>();
    config->use_lstm_nodes = true;
    config->recurrent_hidden_nodes = true;
    config->recurrent_output_nodes = true;
    config->preserve_connectivity = true;

    population = factory->create(*config, *domain);
    CHECK(population);
  }

  unique_ptr<neat::Config> config;
  unique_ptr<DummyDomain> domain;
  unique_ptr<darwin::Population> population;
};

TEST_F(NeatTest, Gene_LoadSave) {
  core_test::TestCaseOutput output;

  neat::Gene gene(0, 1, 0.5f, 100);
  gene.enabled = false;
  gene.recurrent = true;
  json json_obj = gene;
  fprintf(output, "%s", json_obj.dump(2).c_str());
  neat::Gene gene_clone = json_obj;
  EXPECT_EQ(gene_clone.in, 0);
  EXPECT_EQ(gene_clone.out, 1);
  EXPECT_EQ(gene_clone.weight, 0.5f);
  EXPECT_EQ(gene_clone.enabled, false);
  EXPECT_EQ(gene_clone.recurrent, true);
}

TEST_F(NeatTest, Genotype) {
  neat::Genotype genotype;
  genotype.createPrimordialSeed();

  constexpr int kTestMutationCount = 1000;
  atomic<neat::Innovation> next_innovation = 0;
  for (int i = 0; i < kTestMutationCount; ++i) {
    genotype.mutate(next_innovation, true);
    genotype.mutate(next_innovation, false);
  }

  json json_obj = genotype.save();
  neat::Genotype loaded_genotype;
  loaded_genotype.load(json_obj);

  EXPECT_EQ(loaded_genotype.nodes_count, genotype.nodes_count);
  EXPECT_EQ(loaded_genotype.genes.size(), genotype.genes.size());
  for (size_t i = 0; i < genotype.genes.size(); ++i) {
    const auto& gene = genotype.genes[i];
    const auto& loaded_gene = loaded_genotype.genes[i];
    EXPECT_EQ(gene.innovation, loaded_gene.innovation);
    EXPECT_EQ(gene.in, loaded_gene.in);
    EXPECT_EQ(gene.out, loaded_gene.out);
    EXPECT_EQ(gene.weight, loaded_gene.weight);
    EXPECT_EQ(gene.enabled, loaded_gene.enabled);
    EXPECT_EQ(gene.recurrent, loaded_gene.recurrent);
  }
}

TEST_F(NeatTest, Crossover) {
  constexpr int kTestPopulationSize = 100;
  constexpr int kTestGenerations = 500;

  random_device rd;
  default_random_engine rnd(rd());
  uniform_int_distribution<size_t> dist_parent(0, kTestPopulationSize - 1);
  uniform_real_distribution<float> dist_preference(0, 1);

  vector<neat::Genotype> population(kTestPopulationSize);
  atomic<neat::Innovation> next_innovation = 0;

  // initial population
  for (auto& genotype : population) {
    genotype.createPrimordialSeed();
  }

  // test generations
  for (int generation = 0; generation < kTestGenerations; ++generation) {
    for (size_t i = 0; i < population.size(); ++i) {
      const size_t parent1_index = dist_parent(rnd);
      const size_t parent2_index = dist_parent(rnd);
      if (parent1_index == i || parent2_index == i) {
        continue;
      }
      const auto& parent1 = population[parent1_index];
      const auto& parent2 = population[parent2_index];
      population[i].inherit(parent1, parent2, dist_preference(rnd));
      population[i].mutate(next_innovation, false);
    }
  }
}

}  // namespace neat_tests
