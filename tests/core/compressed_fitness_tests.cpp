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

#include <core/darwin.h>
#include <core/evolution.h>
#include <third_party/gtest/gtest.h>

#include <math.h>
#include <algorithm>
#include <vector>
using namespace std;

namespace compressed_fitness_tests {

class TestGenotype : public darwin::Genotype {
  unique_ptr<darwin::Brain> grow() const override { FATAL("Not implemented"); }

  unique_ptr<darwin::Genotype> clone() const override { FATAL("Not implemented"); }

  json save() const override { FATAL("Not implemented"); }

  void load(const json&) override { FATAL("Not implemented"); }
};

class TestPopulation : public darwin::Population {
 public:
  explicit TestPopulation(const vector<float>& fitness_values) {
    CHECK(!fitness_values.empty());
    genotypes_.resize(fitness_values.size());
    for (size_t i = 0; i < genotypes_.size(); ++i) {
      genotypes_[i].fitness = fitness_values[i];
    }
  }

  size_t size() const override { return genotypes_.size(); }

  darwin::Genotype* genotype(size_t index) override { return &genotypes_[index]; }

  const darwin::Genotype* genotype(size_t index) const override {
    return &genotypes_[index];
  }

  vector<size_t> rankingIndex() const override {
    vector<size_t> ranking_index(genotypes_.size());
    for (size_t i = 0; i < ranking_index.size(); ++i) {
      ranking_index[i] = i;
    }
    // sort results by fitness (descending order)
    std::sort(ranking_index.begin(), ranking_index.end(), [&](size_t a, size_t b) {
      return genotypes_[a].fitness > genotypes_[b].fitness;
    });
    return ranking_index;
  }

  int generation() const override { FATAL("Not implemented"); }
  void createPrimordialGeneration(int) override { FATAL("Not implemented"); }
  void createNextGeneration() override { FATAL("Not implemented"); }

 private:
  vector<TestGenotype> genotypes_;
};

TEST(CompressedFitnessTest, SingleValue) {
  constexpr float kTestValue = -125.1f;
  TestPopulation population({ kTestValue });
  auto compressed = darwin::compressFitness(&population);
  EXPECT_EQ(compressed.size(), 1);
  EXPECT_EQ(compressed[0].index, 0);
  EXPECT_EQ(compressed[0].value, kTestValue);
}

TEST(CompressedFitnessTest, TwoValues) {
  constexpr float kTestValue = 100;

  TestPopulation population({ kTestValue, kTestValue });
  auto compressed = darwin::compressFitness(&population);

  // ends are always included
  EXPECT_EQ(compressed.size(), 2);
  EXPECT_EQ(compressed[0].index, 0);
  EXPECT_EQ(compressed[0].value, kTestValue);
  EXPECT_EQ(compressed[1].index, 1);
  EXPECT_EQ(compressed[1].value, kTestValue);
}

TEST(CompressedFitnessTest, ThreeValues1) {
  constexpr float kTestValue = 100;

  TestPopulation population({ kTestValue, kTestValue, kTestValue });
  auto compressed = darwin::compressFitness(&population);

  // ends are always included
  EXPECT_EQ(compressed.size(), 2);
  EXPECT_EQ(compressed[0].index, 0);
  EXPECT_EQ(compressed[0].value, kTestValue);
  EXPECT_EQ(compressed[1].index, 2);
  EXPECT_EQ(compressed[1].value, kTestValue);
}

TEST(CompressedFitnessTest, ThreeValues2) {
  TestPopulation population({ 100, 10, 10 });
  auto compressed = darwin::compressFitness(&population);

  // ends are always included
  EXPECT_EQ(compressed.size(), 3);
  EXPECT_EQ(compressed[0].index, 0);
  EXPECT_EQ(compressed[0].value, 100);
  EXPECT_EQ(compressed[1].index, 1);
  EXPECT_EQ(compressed[1].value, 10);
  EXPECT_EQ(compressed[2].index, 2);
  EXPECT_EQ(compressed[2].value, 10);
}

TEST(CompressedFitnessTest, EqualValues) {
  constexpr float kTestValue = 0.0125f;
  constexpr int kSize = 1000;

  vector<float> fitness_values(kSize);
  for (auto& value : fitness_values)
    value = kTestValue;

  TestPopulation population(fitness_values);
  auto compressed = darwin::compressFitness(&population);

  // ends are always included
  EXPECT_EQ(compressed.size(), 2);
  EXPECT_EQ(compressed[0].index, 0);
  EXPECT_EQ(compressed[0].value, kTestValue);
  EXPECT_EQ(compressed[1].index, kSize - 1);
  EXPECT_EQ(compressed[1].value, kTestValue);
}

TEST(CompressedFitnessTest, SparseValues) {
  constexpr int kSize = 100;
  vector<float> fitness_values(kSize);
  for (int i = 0; i < kSize; ++i)
    fitness_values[i] = pow(2.0f, i);

  TestPopulation population(fitness_values);
  auto compressed = darwin::compressFitness(&population);
  EXPECT_EQ(compressed.size(), kSize);
}

TEST(CompressedFitnessTest, StepValues) {
  constexpr int kSize = 10000;
  constexpr int kStepSize = 1000;
  vector<float> fitness_values(kSize);
  for (int i = 0; i < kSize; ++i)
    fitness_values[i] = i / kStepSize;

  TestPopulation population(fitness_values);
  auto compressed = darwin::compressFitness(&population);
  EXPECT_EQ(compressed.size(), 2 * (kSize / kStepSize));
}

TEST(CompressedFitnessTest, GradientValues) {
  constexpr int kSize = 10000;
  vector<float> fitness_values(kSize);
  for (int i = 0; i < kSize; ++i)
    fitness_values[i] = i;

  TestPopulation population(fitness_values);
  auto compressed = darwin::compressFitness(&population);
  EXPECT_EQ(compressed.size(), 2);
}

}  // namespace compressed_fitness_tests
