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

#include <domains/unicycle/unicycle.h>
#include <domains/unicycle/world.h>

#include <core/darwin.h>
#include <tests/domains/test_brain.h>
#include <third_party/gtest/gtest.h>

#include <memory>
#include <vector>
using namespace std;

namespace unicycle_tests {

struct TestBrain : public core_test::TestBrain {
  const float torque_value = 0;

  TestBrain(float torque_value, size_t inputs_count, size_t outputs_count)
      : core_test::TestBrain(inputs_count, outputs_count), torque_value(torque_value) {}

  void setTestOutputs() override {
    EXPECT_EQ(outputs.size(), 1);
    outputs[0] = torque_value;
  }
};

struct TestGenotype : public darwin::Genotype {
  float torque_value = 0;
  const unicycle::Unicycle* domain = nullptr;

  unique_ptr<darwin::Brain> grow() const override {
    return make_unique<TestBrain>(torque_value, domain->inputs(), domain->outputs());
  }

  unique_ptr<darwin::Genotype> clone() const override { FATAL("Not implemented"); }
  json save() const override { FATAL("Not implemented"); }
  void load(const json&) override { FATAL("Not implemented"); }
};

struct TestPopulation : public darwin::Population {
  TestPopulation(const unicycle::Unicycle* domain, const vector<float>& torque_values) {
    CHECK(!torque_values.empty());
    genotypes_.resize(torque_values.size());
    for (size_t i = 0; i < genotypes_.size(); ++i) {
      genotypes_[i].torque_value = torque_values[i];
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

TEST(UnicycleTest, World_PoleGravity) {
  unicycle::Config config;
  config.max_initial_angle = 15.0f;
  config.max_angle = 45.0f;
  config.max_steps = 100;
  unicycle::Unicycle unicycle(config);

  auto simulation = [&](float initial_angle) -> int {
    unicycle::World world(initial_angle, 0.0f, &unicycle);
    int step = 0;
    for (; step < config.max_steps; ++step) {
      if (!world.simStep())
        break;
    }
    EXPECT_GT(step, 0);
    EXPECT_GT(world.fitnessBonus(), 0);
    if (initial_angle == 0.0f) {
      EXPECT_EQ(world.fitnessBonus(), config.max_steps);
    }
    return step;
  };

  EXPECT_EQ(simulation(0.0f), config.max_steps);
  EXPECT_LT(simulation(+config.max_initial_angle), config.max_steps);
  EXPECT_LT(simulation(-config.max_initial_angle), config.max_steps);
}

TEST(UnicycleTest, World_PoleInertia) {
  unicycle::Config config;
  config.discrete_controls = false;
  config.max_angle = 45.0f;
  config.max_steps = 250;
  config.max_distance = 1024.0f;
  unicycle::Unicycle unicycle(config);

  auto simulation = [&](float torque) -> int {
    unicycle::World world(0.0f, unicycle.randomTargetPosition(), &unicycle);
    int step = 0;
    for (; step < config.max_steps; ++step) {
      world.turnWheel(torque);
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

TEST(UnicycleTest, EvaluatePopulation_SingleInput) {
  constexpr int kMaxSteps = 100;

  unicycle::Config config;
  config.max_steps = kMaxSteps;
  config.max_torque = 5.0f;
  config.test_worlds = 2;
  config.discrete_controls = true;
  config.input_pole_angle = false;
  config.input_angular_velocity = false;
  config.input_wheel_distance = false;
  config.input_wheel_velocity = false;
  config.input_distance_from_target = true;

  unicycle::Unicycle unicycle(config);
  TestPopulation population(&unicycle, { 0.0f, +100.0f, -0.01f, 1.123e30f, -1.0e-40f });
  unicycle.evaluatePopulation(&population);

  for (size_t i = 0; i < population.size(); ++i) {
    EXPECT_GT(population[i]->fitness, 0);
    EXPECT_LE(population[i]->fitness, kMaxSteps);
  }
}

TEST(UnicycleTest, EvaluatePopulation_EveryInput) {
  constexpr int kMaxSteps = 250;

  unicycle::Config config;
  config.max_initial_angle = 0.0f;
  config.max_steps = kMaxSteps;
  config.max_torque = 5.0f;
  config.test_worlds = 3;
  config.discrete_controls = false;
  config.input_pole_angle = true;
  config.input_angular_velocity = true;
  config.input_wheel_distance = true;
  config.input_wheel_velocity = true;
  config.input_distance_from_target = true;
  
  vector<float> torque_values(5);
  torque_values[0] = 0.0f;
  torque_values[1] = +0.5f;
  torque_values[2] = -0.5f;
  torque_values[3] = +1.0f;
  torque_values[4] = -1.0f;

  unicycle::Unicycle unicycle(config);
  TestPopulation population(&unicycle, torque_values);
  unicycle.evaluatePopulation(&population);

  // torque = 0.0f
  EXPECT_GE(population[0]->fitness, 1.0f);
  
  // torque = +/-0.5f
  EXPECT_GT(population[0]->fitness, population[1]->fitness);
  EXPECT_EQ(population[1]->fitness, population[2]->fitness);

  // torque = +/-1.0f
  EXPECT_GT(population[2]->fitness, population[3]->fitness);
  EXPECT_EQ(population[3]->fitness, population[4]->fitness);
  EXPECT_GT(population[4]->fitness, 0);
}

}  // namespace unicycle_tests
