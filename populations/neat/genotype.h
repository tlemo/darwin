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

#include "neat.h"

#include <core/darwin.h>

#include <array>
#include <unordered_set>
#include <vector>
using namespace std;

namespace neat {

enum LstmWeightIds { Wi, Ui, Bi, Wf, Uf, Bf, Wo, Uo, Bo, Wc, Uc, Bc, Nlstm };

using LstmWeights = array<float, Nlstm>;

// NOTE: nodes are implicitly numbered as follows:
//  bias node    : 0
//  input nodes  : [1, INPUTS]
//  output nodes : [INPUTS + 1, INPUTS + OUTPUTS]
//  hidden nodes : [INPUTS + OUTPUTS + 1, nodes_count)

using NodeId = size_t;
using Innovation = size_t;

constexpr NodeId kBiasNodeId = 0;
constexpr NodeId kFirstInput = 1;

// a NEAT gene represents one link in the ANN
struct Gene {
  Innovation innovation = 0;
  NodeId in = 0;
  NodeId out = 0;
  float weight = 0;
  bool enabled = true;
  bool recurrent = false;

  Gene(NodeId in, NodeId out, float weight, Innovation innovation)
      : innovation(innovation), in(in), out(out), weight(weight) {}

  Gene() = default;

  friend void to_json(json& json_obj, const Gene& gene);
  friend void from_json(const json& json_obj, Gene& gene);
};

class Genotype : public darwin::Genotype {
  // the "white/gray/black" DFS colors
  enum class VisitedColor {
    NotYetVisited,
    Exploring,
    Visited,
  };

 public:
  vector<Gene> genes;
  size_t nodes_count = 0;  // total, including bias/in/out/hidden

  LstmWeights lw = {};

  int age = 0;

  Genotype();

  unique_ptr<darwin::Genotype> clone() const override;

  void reset() override;

  // generate an initial genotype with no hidden nodes
  // (we generate a fully connected network with random weights)
  //
  // NOTE: there's an implicit contract that the innovation values
  //   are consistent for the generated genes
  //
  Innovation createPrimordialSeed();

  void mutate(atomic<Innovation>& next_innovation, bool weights_only = false);

  // combine the genes from two parents, renumbering the hidden nodes
  void inherit(const Genotype& parent1, const Genotype& parent2, float preference);

  // calculates the compatibility distance between
  // this genotype and a reference one
  // (as described in the NEAT paper)
  double compatibility(const Genotype& ref) const;

  unique_ptr<darwin::Brain> grow() const override;

  json save() const override;
  void load(const json& json_obj) override;

 private:
  template <class RND>
  void mutateWeights(RND& rnd) {
    std::bernoulli_distribution dist_mutate(g_config.weight_mutation_chance);

    // CONSIDER: trimming (disabling?) links with weight < epsilon?
    for (auto& gene : genes)
      if (dist_mutate(rnd))
        ann::mutateValue(gene.weight, rnd, ann::g_config.mutation_std_dev);

    if (g_config.use_lstm_nodes) {
      for (float& w : lw)
        if (dist_mutate(rnd))
          ann::mutateValue(w, rnd, ann::g_config.mutation_std_dev);
    }
  }

  // returns true if we can reach dst from src (or if dst == src)
  // using only non-recurrent links (so traversing a DAG, no cycles)
  bool canReach(NodeId src, NodeId dst, unordered_set<NodeId>& visited) const;

  // returns true if it detects any cycles (using node_id as search root)
  bool detectCycles(NodeId node_id, vector<VisitedColor>& visited) const;

  template <class RND>
  void mutateNewLinks(RND& rnd, atomic<Innovation>& next_innovation) {
    const NodeId kInputFirst = 1;
    const NodeId kOutputFirst = 1 + g_inputs;

    std::bernoulli_distribution dist_mutate(g_config.new_link_chance);
    if (dist_mutate(rnd)) {
      std::uniform_int_distribution<NodeId> dist_in_node(kInputFirst, nodes_count - 1);
      std::uniform_int_distribution<NodeId> dist_out_node(kOutputFirst, nodes_count - 1);

      NodeId in = dist_in_node(rnd);
      NodeId out = dist_out_node(rnd);

      const float range = ann::g_config.connection_range;
      std::uniform_real_distribution<float> dist_weight(-range, range);

      // check to see if the link already exists
      for (Gene& gene : genes)
        if (gene.in == in && gene.out == out) {
          // re-enable and mutate disabled links
          if (!gene.enabled) {
            gene.enabled = true;
            gene.weight = ann::roundWeight(dist_weight(rnd));
          }
          return;
        }

      // create a new link
      Gene new_gene(in, out, ann::roundWeight(dist_weight(rnd)), next_innovation++);
      unordered_set<NodeId> visited;
      new_gene.recurrent = canReach(out, in, visited);
      genes.push_back(new_gene);
    }
  }

  template <class RND>
  void mutateNewNodes(RND& rnd, atomic<Innovation>& next_innovation) {
    std::bernoulli_distribution dist_mutate(g_config.new_node_chance);
    if (dist_mutate(rnd)) {
      // pick a random gene to split
      std::uniform_int_distribution<size_t> dist_gene_index(0, genes.size() - 1);
      auto& gene = genes[dist_gene_index(rnd)];

      const float range = ann::g_config.connection_range;
      std::uniform_real_distribution<float> dist_weight(-range, range);

      NodeId new_node_id = nodes_count++;
      Gene pre_link(gene.in, new_node_id, 1.0f, next_innovation++);
      pre_link.recurrent = false;
      Gene post_link(new_node_id, gene.out, gene.weight, next_innovation++);
      post_link.recurrent = gene.recurrent;
      gene.enabled = false;
      genes.push_back(pre_link);
      genes.push_back(post_link);

      if (g_config.implicit_bias_links) {
        Gene bias(kBiasNodeId,
                  new_node_id,
                  ann::roundWeight(dist_weight(rnd)),
                  next_innovation++);
        genes.push_back(bias);
      }

      if (g_config.recurrent_hidden_nodes) {
        Gene self_link(new_node_id,
                       new_node_id,
                       ann::roundWeight(dist_weight(rnd)),
                       next_innovation++);
        self_link.recurrent = true;
        genes.push_back(self_link);
      }
    }
  }
};

}  // namespace neat
