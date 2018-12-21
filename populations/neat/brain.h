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

#pragma once

#include "genotype.h"
#include "neat.h"

#include <core/ann_activation_functions.h>
#include <core/darwin.h>

namespace neat {

struct Link {
  NodeId in;
  float weight;
  bool recurrent;

  Link(NodeId in, float weight, bool recurrent)
      : in(in), weight(weight), recurrent(recurrent) {}
};

struct Node {
  virtual ~Node() = default;

  float value = 0;
  vector<Link> inputs;

  virtual void resetState() { value = 0; }
  virtual void activate(float input) { value = ann::activate(input); }
};

struct LstmNode : public Node {
  LstmNode(const array<float, Nlstm>& lw) : lw(lw) {}

  // LSTM weights (points directly into the genotype)
  const array<float, Nlstm>& lw;

  // LSTM state
  float cell = 0;

  void resetState() override;

  void activate(float input) override;
};

// Phenotype
class Brain : public darwin::Brain {
  // see the genotype comments regarding the node numbering
  constexpr static int kFirstInput = 1;

 public:
  Brain(const Genotype* genotype);

  // index is the input index [0, INPUTS)
  void setInput(int index, float value) override {
    CHECK(index < g_inputs);
    nodes_[kFirstInput + index]->value = value;
  }

  // index is the output index [0, OUTPUTS)
  float output(int index) const override {
    CHECK(index < g_outputs);
    return nodes_[kFirstInput + g_inputs + index]->value;
  }

  void think() override;

  void resetState() override {
    for (const auto& node : nodes_)
      node->resetState();
  }

 private:
  vector<unique_ptr<Node>> nodes_;
  vector<NodeId> eval_order_;
};

}  // namespace neat
