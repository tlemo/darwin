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

#include <domains/cart_pole/cart_pole.h>
#include <domains/cart_pole/world.h>

#include <core/darwin.h>
#include <third_party/gtest/gtest.h>

#include <memory>
#include <vector>
#include <unordered_set>
using namespace std;

namespace cart_pole_tests {

struct TestBrain : public darwin::Brain {
  const float force_value = 0;
  const size_t inputs = 0;
  const size_t outputs = 0;

  // model the expected usage pattern:
  //
  // for_each(episode_step):
  //    1. for_each(input_index): setInput(input_index, value)
  //    2. think()
  //    3. for_each(ouput_index): output(output_index)
  //
  enum class State { WaitingForInputs, OutputsReady };
  State state = State::WaitingForInputs;

  // make sure all the inputs and outputs are used
  mutable unordered_set<int> used_inputs;
  mutable unordered_set<int> used_outputs;

  TestBrain(float force_value, size_t inputs, size_t outputs)
      : force_value(force_value), inputs(inputs), outputs(outputs) {}

  ~TestBrain() { checkOutputsConsumed(); }

  void checkInputsSet() {
    EXPECT_EQ(state, State::WaitingForInputs);
    EXPECT_EQ(used_inputs.size(), inputs);
  }

  void checkOutputsConsumed() {
    EXPECT_EQ(state, State::OutputsReady);
    EXPECT_EQ(used_outputs.size(), outputs);
  }

  void setInput(int index, float) override {
    if (state == State::OutputsReady) {
      checkOutputsConsumed();
      used_inputs.clear();
      used_outputs.clear();
      state = State::WaitingForInputs;
    }
    EXPECT_EQ(state, State::WaitingForInputs);
    EXPECT_GE(index, 0);
    EXPECT_LT(index, inputs);
    EXPECT_TRUE(used_inputs.insert(index).second);
  }

  float output(int index) const override {
    EXPECT_EQ(state, State::OutputsReady);
    EXPECT_GE(index, 0);
    EXPECT_LT(index, outputs);
    EXPECT_TRUE(used_outputs.insert(index).second);
    return force_value;
  }

  void think() override {
    checkInputsSet();
    state = State::OutputsReady;
  }

  void resetState() override { FATAL("Not implemented"); }
};

struct TestGenotype : public darwin::Genotype {
  float force_value = 0;
  const cart_pole::CartPole* domain = nullptr;

  unique_ptr<darwin::Brain> grow() const override {
    return make_unique<TestBrain>(force_value, domain->inputs(), domain->outputs());
  }

  unique_ptr<darwin::Genotype> clone() const override { FATAL("Not implemented"); }

  json save() const override { FATAL("Not implemented"); }

  void load(const json&) override { FATAL("Not implemented"); }
};

struct TestPopulation : public darwin::Population {
  TestPopulation(const cart_pole::CartPole* domain, const vector<float>& force_values) {
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

  void rankGenotypes() override { FATAL("Not implemented"); }

  int generation() const override { return 0; }

  void createPrimordialGeneration(int) override { FATAL("Not implemented"); }

  void createNextGeneration() override { FATAL("Not implemented"); }

 private:
  vector<TestGenotype> genotypes_;
};

TEST(CartPoleTest, World_PoleGravity) {
  cart_pole::Config config;
  config.max_initial_angle = 15.0f;
  config.max_angle = 60.0f;
  config.max_steps = 100;
  cart_pole::CartPole cart_pole(config);

  auto simulation = [&](float initial_angle) -> int {
    cart_pole::World world(initial_angle, &cart_pole);
    int step = 0;
    for (; step < config.max_steps; ++step) {
      if (!world.simStep())
        break;
    }
    return step;
  };

  EXPECT_EQ(simulation(0.0f), config.max_steps);
  EXPECT_LT(simulation(+config.max_initial_angle), config.max_steps);
  EXPECT_LT(simulation(-config.max_initial_angle), config.max_steps);
}

TEST(CartPoleTest, World_PoleInertia) {
  cart_pole::Config config;
  config.discrete_controls = false;
  config.max_angle = 60.0f;
  config.max_steps = 250;
  config.max_distance = 1.0e10f;
  cart_pole::CartPole cart_pole(config);

  auto simulation = [&](float force) -> int {
    cart_pole::World world(0.0f, &cart_pole);
    int step = 0;
    for (; step < config.max_steps; ++step) {
      world.moveCart(force);
      if (!world.simStep())
        break;
    }
    return step;
  };

  EXPECT_EQ(simulation(0.0f), config.max_steps);
  EXPECT_LT(simulation(+1.0f), config.max_steps);
  EXPECT_LT(simulation(-1.0f), config.max_steps);
}

TEST(CartPoleTest, EvaluatePopulation_SingleInput) {
  constexpr int kMaxSteps = 100;

  cart_pole::Config config;
  config.max_steps = kMaxSteps;
  config.max_force = 5.0f;
  config.test_worlds = 2;
  config.discrete_controls = true;
  config.input_pole_angle = false;
  config.input_angular_velocity = true;
  config.input_cart_distance = false;
  config.input_cart_velocity = false;

  cart_pole::CartPole cart_pole(config);
  TestPopulation population(&cart_pole, { 0.0f, +100.0f, -0.01f, 1.123e30f, -1.0e-40f });
  cart_pole.evaluatePopulation(&population);

  for (size_t i = 0; i < population.size(); ++i) {
    EXPECT_GT(population[i]->fitness, 0);
    EXPECT_LE(population[i]->fitness, kMaxSteps);
  }
}

TEST(CartPoleTest, EvaluatePopulation_EveryInput) {
  constexpr int kMaxSteps = 250;

  cart_pole::Config config;
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

  cart_pole::CartPole cart_pole(config);
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

}  // namespace cart_pole_tests
