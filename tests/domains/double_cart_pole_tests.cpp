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

#include <domains/double_cart_pole/double_cart_pole.h>
#include <domains/double_cart_pole/world.h>

#include <core/darwin.h>
#include <tests/domains/test_brain.h>
#include <third_party/gtest/gtest.h>

#include <memory>
#include <vector>
using namespace std;

namespace double_cart_pole_tests {

struct TestBrain : public core_test::TestBrain {
  const float force_value = 0;

  TestBrain(float force_value, size_t inputs_count, size_t outputs_count)
      : core_test::TestBrain(inputs_count, outputs_count), force_value(force_value) {}
      
  void setTestOutputs() override {
    EXPECT_EQ(outputs.size(), 1);
    outputs[0] = force_value;
  }
};

struct TestGenotype : public darwin::Genotype {
  float force_value = 0;
  const double_cart_pole::DoubleCartPole* domain = nullptr;

  unique_ptr<darwin::Brain> grow() const override {
    return make_unique<TestBrain>(force_value, domain->inputs(), domain->outputs());
  }

  unique_ptr<darwin::Genotype> clone() const override { FATAL("Not implemented"); }

  json save() const override { FATAL("Not implemented"); }

  void load(const json&) override { FATAL("Not implemented"); }
};

struct TestPopulation : public darwin::Population {
  TestPopulation(const double_cart_pole::DoubleCartPole* domain, const vector<float>& force_values) {
    CHECK(!force_values.empty());
    genotypes_.resize(force_values.size());
    for (size_t i = 0; i < genotypes_.size(); ++i) {
      genotypes_[i].force_value = force_values[i];
      genotypes_[i].domain = domain;
    }
  }

  size_t size() const override { return genotypes_.size(); }

  darwin::Genotype* genotype(size_t index) override { return &genotypes_[index]; }

  const darwin::Genotype* genotype(size_t index) const override {
    return &genotypes_[index];
  }

  int generation() const override { return 0; }

  void rankGenotypes() override { FATAL("Not implemented"); }
  void createPrimordialGeneration(int) override { FATAL("Not implemented"); }
  void createNextGeneration() override { FATAL("Not implemented"); }

 private:
  vector<TestGenotype> genotypes_;
};

TEST(DoubleCartPoleTest, World_PoleGravity) {
  double_cart_pole::Config config;
  config.max_initial_angle = 15.0f;
  config.max_angle = 60.0f;
  config.max_steps = 100;
  double_cart_pole::DoubleCartPole cart_pole(config);

  auto simulation = [&](float initial_angle_1, float initial_angle_2) -> int {
    double_cart_pole::World world(initial_angle_1, initial_angle_2, &cart_pole);
    int step = 0;
    for (; step < config.max_steps; ++step) {
      if (!world.simStep())
        break;
    }
    EXPECT_GT(step, 0);
    return step;
  };

  const float kAngle = config.max_initial_angle;
  
  EXPECT_EQ(simulation(0.0f, 0.0f), config.max_steps);
  EXPECT_LT(simulation(+kAngle, 0.0f), config.max_steps);
  EXPECT_LT(simulation(0.0f, -kAngle), config.max_steps);
  EXPECT_LT(simulation(+kAngle, +kAngle), config.max_steps);
  EXPECT_LT(simulation(-kAngle, -kAngle), config.max_steps);
  EXPECT_LT(simulation(+kAngle, -kAngle), config.max_steps);
  EXPECT_LT(simulation(-kAngle, +kAngle), config.max_steps);
}

TEST(DoubleCartPoleTest, World_PoleInertia) {
  double_cart_pole::Config config;
  config.discrete_controls = false;
  config.max_angle = 60.0f;
  config.max_steps = 250;
  config.max_distance = 1.0e10f;
  double_cart_pole::DoubleCartPole cart_pole(config);

  auto simulation = [&](float force) -> int {
    double_cart_pole::World world(0.0f, 0.0f, &cart_pole);
    int step = 0;
    for (; step < config.max_steps; ++step) {
      world.moveCart(force);
      if (!world.simStep())
        break;
    }
    EXPECT_GT(step, 0);
    return step;
  };

  EXPECT_EQ(simulation(0.0f), config.max_steps);
  EXPECT_LT(simulation(+1.0f), config.max_steps);
  EXPECT_LT(simulation(-1.0f), config.max_steps);
}

TEST(DoubleCartPoleTest, EvaluatePopulation_SingleInput) {
  constexpr int kMaxSteps = 100;

  double_cart_pole::Config config;
  config.max_steps = kMaxSteps;
  config.max_force = 5.0f;
  config.test_worlds = 2;
  config.discrete_controls = true;
  config.input_pole_angle = true;
  config.input_angular_velocity = false;
  config.input_cart_distance = false;
  config.input_cart_velocity = false;

  double_cart_pole::DoubleCartPole cart_pole(config);
  TestPopulation population(&cart_pole, { 0.0f, +100.0f, -0.001f, 0.123e10f, -1.0e-20f });
  cart_pole.evaluatePopulation(&population);

  for (size_t i = 0; i < population.size(); ++i) {
    EXPECT_GT(population[i]->fitness, 0);
    EXPECT_LE(population[i]->fitness, kMaxSteps);
  }
}

TEST(DoubleCartPoleTest, EvaluatePopulation_EveryInput) {
  constexpr int kMaxSteps = 250;

  double_cart_pole::Config config;
  config.max_initial_angle = 0.0f;
  config.max_steps = kMaxSteps;
  config.max_force = 5.0f;
  config.test_worlds = 3;
  config.discrete_controls = false;
  config.input_pole_angle = true;
  config.input_angular_velocity = true;
  config.input_cart_distance = true;
  config.input_cart_velocity = true;
  
  vector<float> force_values(5);
  force_values[0] = 0.0f;
  force_values[1] = +1.0f;
  force_values[2] = -1.0f;
  force_values[3] = +2.0f;
  force_values[4] = -2.0f;

  double_cart_pole::DoubleCartPole cart_pole(config);
  TestPopulation population(&cart_pole, force_values);
  cart_pole.evaluatePopulation(&population);

  // force = 0.0f
  EXPECT_EQ(population[0]->fitness, kMaxSteps);
  
  // force = +/-1.0f
  EXPECT_GT(population[0]->fitness, population[1]->fitness);
  EXPECT_EQ(population[1]->fitness, population[2]->fitness);

  // force = +/-2.0f
  EXPECT_GT(population[2]->fitness, population[3]->fitness);
  EXPECT_EQ(population[3]->fitness, population[4]->fitness);
  EXPECT_GT(population[4]->fitness, 0);
}

}  // namespace double_cart_pole_tests
