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

#include "brain.h"
#include "dummy.h"
#include "population.h"

#include <algorithm>
using namespace std;

namespace dummy {

Brain::Brain(const Genotype* genotype) : genotype_(genotype) {
  auto domain = genotype_->population()->domain();
  inputs_.resize(domain->inputs());
  outputs_.resize(domain->outputs());
  used_inputs_.resize(inputs_.size());
  used_outputs_.resize(outputs_.size());
  resetState();
}

Brain::~Brain() {
  if (state_ == State::OutputsReady) {
    checkOutputsConsumed();
  }
}

void Brain::setInput(int index, float value) {
  if (state_ == State::OutputsReady) {
    checkOutputsConsumed();
    resetUsedFlags();
  }
  CHECK(state_ == State::WaitingForInputs);
  CHECK(index >= 0 && index < int(inputs_.size()));
  CHECK(!used_inputs_[index]);
  used_inputs_[index] = true;
  inputs_[index] = value;
}

float Brain::output(int index) const {
  CHECK(state_ == State::OutputsReady);
  CHECK(index >= 0 && index < int(outputs_.size()));
  used_outputs_[index] = true;
  return outputs_[index];
}

void Brain::think() {
  checkInputsSet();

  // generate output values
  const auto& config = genotype_->population()->config();
  for (float& value : outputs_) {
    uniform_real_distribution<float> dist(-config.output_range, config.output_range);
    value = config.random_outputs ? dist(rnd_) : config.const_output;
  }

  state_ = State::OutputsReady;
}

void Brain::resetState() {
  if (state_ == State::OutputsReady) {
    checkOutputsConsumed();
  }
  rnd_.seed(genotype_->seed());
  resetUsedFlags();
}

void Brain::resetUsedFlags() {
  std::fill(used_inputs_.begin(), used_inputs_.end(), false);
  std::fill(used_outputs_.begin(), used_outputs_.end(), false);
  state_ = State::WaitingForInputs;
}

void Brain::checkInputsSet() {
  CHECK(state_ == State::WaitingForInputs);
  
  for(bool used : used_inputs_) {
    CHECK(used);
  }

  const auto& config = genotype_->population()->config();
  for (float value : inputs_) {
    CHECK(value >= -config.input_range);
    CHECK(value <= config.input_range);
  }
}

void Brain::checkOutputsConsumed() {
  CHECK(state_ == State::OutputsReady);
  for(bool used : used_outputs_) {
    CHECK(used);
  }
}

}  // namespace dummy
