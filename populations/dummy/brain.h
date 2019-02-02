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

#include "genotype.h"

#include <core/darwin.h>

#include <random>
#include <vector>
using namespace std;

namespace dummy {

class Brain : public darwin::Brain {
  // model and check the expected usage pattern:
  //
  // for_each(episode_step):
  //    1. for_each(input_index): setInput(input_index, value)
  //    2. think()
  //    3. for_each(ouput_index): output(output_index)
  //
  enum class State { WaitingForInputs, OutputsReady };

 public:
  explicit Brain(const Genotype* genotype);
  ~Brain();

  void setInput(int index, float value) override;
  float output(int index) const override;
  void think() override;
  void resetState() override;

 private:
  void resetUsedFlags();
  void checkInputsSet();
  void checkOutputsConsumed();
  
 private:
  const Genotype* genotype_ = nullptr;
  default_random_engine rnd_;

  State state_ = State::WaitingForInputs;
  
  // input & output values
  vector<float> inputs_;
  vector<float> outputs_;
  
  // make sure all the inputs and outputs are used
  mutable vector<char> used_inputs_;
  mutable vector<char> used_outputs_;
};

}  // namespace dummy
