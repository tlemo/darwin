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

#include "brain.h"

#include <queue>
using namespace std;

namespace neat {

void LstmNode::resetState() {
  value = 0;
  cell = 0;
}

void LstmNode::activate(float input) {
  float cand_C = ann::activate(lw[Wc] * input + lw[Uc] * value + lw[Bc]);
  float i_gate = ann::activateGate(lw[Wi] * input + lw[Ui] * value + lw[Bi]);
  float f_gate = ann::activateGate(lw[Wf] * input + lw[Uf] * value + lw[Bf]);
  float o_gate = ann::activateGate(lw[Wo] * input + lw[Uo] * value + lw[Bo]);
  cell = f_gate * cell + i_gate * cand_C;
  value = o_gate * ann::activate(cell);
}

void Brain::synthesize(const Genotype* genotype) {
  const int kFirstOutput = kFirstInput + g_inputs;

  CHECK(g_inputs > 0);
  CHECK(g_outputs > 0);
  CHECK(genotype->nodes_count >= kFirstOutput + g_outputs);

  nodes_ = vector<unique_ptr<Node>>(genotype->nodes_count);
  for (auto& node : nodes_) {
    node = g_config.use_lstm_nodes ? make_unique<LstmNode>(genotype->lw)
                                   : make_unique<Node>();
  }

  for (const auto& gene : genotype->genes) {
    CHECK(gene.out != kBiasNodeId);
    CHECK(gene.in < nodes_.size());
    CHECK(gene.out < nodes_.size());
    nodes_[gene.out]->inputs.emplace_back(gene.in, gene.weight, gene.recurrent);
  }

  // topological sort, for evaluation order
  // (calculate the _reverse_ topological sort since we only track in arcs)
  vector<int> outs_count(nodes_.size());
  for (const auto& node : nodes_)
    for (const auto& link : node->inputs) {
      if (!link.recurrent)
        ++outs_count[link.in];
    }

  eval_order_.clear();

  queue<NodeId> top_queue;
  for (NodeId i = 0; i < outs_count.size(); ++i)
    if (outs_count[i] == 0)
      top_queue.push(i);
  CHECK(!top_queue.empty(), "can't topsort the network");

  while (!top_queue.empty()) {
    NodeId node_id = top_queue.front();
    top_queue.pop();

    // bias & input nodes must NOT be be evaluated
    if (node_id >= kFirstOutput)
      eval_order_.push_back(node_id);

    for (const auto& link : nodes_[node_id]->inputs)
      if (!link.recurrent) {
        CHECK(outs_count[link.in] > 0);
        if (--outs_count[link.in] == 0)
          top_queue.push(link.in);
      }
  }

  CHECK(eval_order_.size() == nodes_.size() - kFirstOutput);
  CHECK(std::count(outs_count.begin(), outs_count.end(), 0) == nodes_.size());

  // order inputs -> ... -> outputs
  std::reverse(eval_order_.begin(), eval_order_.end());
}

void Brain::think() {
  const int kFirstOutput = kFirstInput + g_inputs;
  const int kFirstHidden = kFirstOutput + g_outputs;

  nodes_[kBiasNodeId]->value = 1.0f;

  if (g_config.normalize_input) {
    for (NodeId i = 0; i < g_inputs; ++i) {
      const auto& node = nodes_[kFirstInput + i];
      node->activate(node->value);
    }
  }

  for (auto node_id : eval_order_) {
    CHECK(node_id >= kFirstOutput);

    const auto& node = nodes_[node_id];
    float value = 0;
    for (const auto& link : node->inputs)
      value += nodes_[link.in]->value * link.weight;

    if (g_config.normalize_output || node_id >= kFirstHidden)
      node->activate(value);
    else
      node->value = value;
  }
}

}  // namespace neat
