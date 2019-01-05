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
  mutable unordered_set<int> input_set;
  mutable unordered_set<int> output_set;
  
  TestBrain(float force_value, size_t inputs, size_t outputs)
      : force_value(force_value), inputs(inputs), outputs(outputs) {}

  ~TestBrain() {
    checkOutputsConsumed();
  }
  
  void checkInputsSet() {
    EXPECT_EQ(state, State::WaitingForInputs);
    EXPECT_EQ(input_set.size(), inputs);
  }
  
  void checkOutputsConsumed() {
    EXPECT_EQ(state, State::OutputsReady);
    EXPECT_EQ(output_set.size(), outputs);
  }

  void setInput(int index, float) override {
    if (state == State::OutputsReady) {
      checkOutputsConsumed();
      state = State::WaitingForInputs;
    }
    EXPECT_EQ(state, State::WaitingForInputs);
    EXPECT_GE(index, 0);
    EXPECT_LT(index, inputs);
    EXPECT_TRUE(input_set.insert(index).second);
  }

  float output(int index) const override {
    EXPECT_EQ(state, State::OutputsReady);
    EXPECT_GE(index, 0);
    EXPECT_LT(index, outputs);
    EXPECT_TRUE(output_set.insert(index).second);
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

  int generation() const override { FATAL("Not implemented"); }

  void createPrimordialGeneration(int) override { FATAL("Not implemented"); }

  void createNextGeneration() override { FATAL("Not implemented"); }

 private:
  vector<TestGenotype> genotypes_;
};

TEST(CartPoleTest, Test) {
  cart_pole::Config config;
  cart_pole::CartPole cart_pole(config);
  TestPopulation population(&cart_pole, { 0.0f, 1.0f, -1.0f, 2.0f, -2.0f });
  EXPECT_FALSE(cart_pole.evaluatePopulation(&population));
}

}  // namespace cart_pole_tests
