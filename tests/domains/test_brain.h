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

#pragma once

#include <core/darwin.h>
#include <third_party/gtest/gtest.h>

#include <unordered_set>
#include <vector>
using namespace std;

namespace core_test {

struct TestBrain : public darwin::Brain {
  // storage for input & output values
  vector<float> inputs;
  vector<float> outputs;

  // model and check the expected usage pattern:
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

  TestBrain(size_t inputs_count, size_t outputs_count)
      : inputs(inputs_count), outputs(outputs_count) {}

  ~TestBrain() { checkOutputsConsumed(); }

  void checkInputsSet() {
    EXPECT_EQ(state, State::WaitingForInputs);
    EXPECT_EQ(used_inputs.size(), inputs.size());
  }

  void checkOutputsConsumed() {
    EXPECT_EQ(state, State::OutputsReady);
    EXPECT_EQ(used_outputs.size(), outputs.size());
  }

  void setInput(int index, float value) override {
    if (state == State::OutputsReady) {
      checkOutputsConsumed();
      used_inputs.clear();
      used_outputs.clear();
      state = State::WaitingForInputs;
    }
    EXPECT_EQ(state, State::WaitingForInputs);
    EXPECT_GE(index, 0);
    EXPECT_LT(index, inputs.size());
    EXPECT_TRUE(used_inputs.insert(index).second);
    inputs[index] = value;
  }

  float output(int index) const override {
    EXPECT_EQ(state, State::OutputsReady);
    EXPECT_GE(index, 0);
    EXPECT_LT(index, outputs.size());
    EXPECT_TRUE(used_outputs.insert(index).second);
    return outputs[index];
  }

  void think() override {
    checkInputsSet();
    setTestOutputs();
    state = State::OutputsReady;
  }
  
  virtual void setTestOutputs() = 0;

  void resetState() override { FATAL("Not implemented"); }
};

}  // namespace core_test
