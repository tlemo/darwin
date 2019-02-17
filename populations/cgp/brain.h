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

#include "cgp.h"
#include "genotype.h"
#include "population.h"
#include "functions.h"

#include <core/darwin.h>

#include <array>
#include <vector>
using namespace std;

namespace cgp {

class Brain : public darwin::Brain {
  struct Instruction {
    FunctionId function;
    array<IndexType, kMaxFunctionArity> sources;
  };

 public:
  explicit Brain(const Genotype* genotype);

  void setInput(int index, float value) override;
  float output(int index) const override;
  void think() override;
  void resetState() override;

 private:
  IndexType dfsNodeEval(IndexType node_index, vector<IndexType>& nodes_map);

 private:
  const Genotype* genotype_ = nullptr;

  vector<Instruction> instructions_;
  vector<float> registers_;
  vector<float> memory_;
  vector<int> outputs_map_;
};

}  // namespace cgp
