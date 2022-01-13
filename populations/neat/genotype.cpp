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

#include "genotype.h"
#include "brain.h"
#include "neat.h"

#include <core/random.h>

namespace neat {

Genotype::Genotype() {
  reset();
}

unique_ptr<darwin::Genotype> Genotype::clone() const {
  return make_unique<Genotype>(*this);
}

void Genotype::reset() {
  darwin::Genotype::reset();

  CHECK(g_inputs > 0);
  CHECK(g_outputs > 0);
  genes.clear();
  nodes_count = g_inputs + g_outputs + 1;  // fixed node IDs
  age = 0;
  lw = {};
}

// DFS search for dst, starting at src
bool Genotype::canReach(NodeId src, NodeId dst, unordered_set<NodeId>& visited) const {
  if (src == dst)
    return true;

  // already visited src?
  // (excluding recurrent links we have a DAG, so multiple paths are possible)
  if (!visited.insert(src).second)
    return false;

  for (const auto& gene : genes) {
    if (!gene.recurrent && gene.in == src && canReach(gene.out, dst, visited))
      return true;
  }

  // can't reach dst from src
  return false;
}

// DFS helper used to detect cycles
bool Genotype::detectCycles(NodeId node_id, vector<VisitedColor>& visited) const {
  switch (visited[node_id]) {
    case VisitedColor::Exploring:
      return true;

    case VisitedColor::Visited:
      return false;

    case VisitedColor::NotYetVisited:
      visited[node_id] = VisitedColor::Exploring;
      for (const auto& gene : genes) {
        if (!gene.recurrent && gene.in == node_id && detectCycles(gene.out, visited))
          return true;
      }
      visited[node_id] = VisitedColor::Visited;
      return false;

    default:
      FATAL("Unexpected visited color");
  }
}

void to_json(nlohmann::json& json_obj, const Gene& gene) {
  json_obj["innovation"] = gene.innovation;
  json_obj["in"] = gene.in;
  json_obj["out"] = gene.out;
  json_obj["weight"] = gene.weight;
  json_obj["enabled"] = gene.enabled;
  json_obj["recurrent"] = gene.recurrent;
}

void from_json(const json& json_obj, Gene& gene) {
  gene.innovation = json_obj.at("innovation");
  gene.in = json_obj.at("in");
  gene.out = json_obj.at("out");
  gene.weight = json_obj.at("weight");
  gene.enabled = json_obj.at("enabled");
  gene.recurrent = json_obj.at("recurrent");
}

json Genotype::save() const {
  json json_obj;
  json_obj["genes"] = genes;
  json_obj["nodes_count"] = nodes_count;
  json_obj["lw"] = lw;
  json_obj["inputs"] = g_inputs;
  json_obj["outputs"] = g_outputs;
  json_obj["lstm"] = g_config.use_lstm_nodes;
  return json_obj;
}

void Genotype::load(const json& json_obj) {
  // load the genotype
  Genotype tmp_genotype;
  tmp_genotype.genes = json_obj.at("genes").get<vector<Gene>>();
  tmp_genotype.nodes_count = json_obj.at("nodes_count");
  tmp_genotype.lw = json_obj.at("lw");

  // check inputs & outputs count
  const int tmp_inputs = json_obj.at("inputs");
  const int tmp_outputs = json_obj.at("outputs");
  if (tmp_inputs != g_inputs || tmp_outputs != g_outputs)
    throw core::Exception("Can't load genotype, mismatched inputs or outputs count");
  if (tmp_genotype.nodes_count < kFirstInput + g_inputs + g_outputs)
    throw core::Exception("Can't load genotype, invalid nodes count");

  // the genotype must be compatible with the current population configuration
  if (json_obj.at("lstm") != g_config.use_lstm_nodes)
    throw core::Exception("Can't load genotype, not matching the population config");

  // check all the node ids
  for (const auto& gene : tmp_genotype.genes) {
    if (gene.in >= tmp_genotype.nodes_count || gene.out >= tmp_genotype.nodes_count)
      throw core::Exception("Can't load genotype, invalid gene");
  }

  // check genotype topology: no cycles (excluding the genes marked as recurrent)
  vector<VisitedColor> visited(tmp_genotype.nodes_count, VisitedColor::NotYetVisited);
  for (NodeId node_id = 0; node_id < tmp_genotype.nodes_count; ++node_id) {
    CHECK(visited[node_id] != VisitedColor::Exploring);
    if (tmp_genotype.detectCycles(node_id, visited))
      throw core::Exception("Can't load genotype, cycle detected");
  }

  std::swap(*this, tmp_genotype);
}

Innovation Genotype::createPrimordialSeed() {
  reset();

  const NodeId kFirstOutput = kFirstInput + g_inputs;

  const float range = ann::g_config.connection_range;

  std::default_random_engine rnd(core::randomSeed());
  std::uniform_real_distribution<float> dist(-range, range);

  Innovation innovation = 1;

  for (int out = 0; out < g_outputs; ++out) {
    for (int in = 0; in < g_inputs; ++in)
      genes.emplace_back(kFirstInput + in,
                         kFirstOutput + out,
                         ann::roundWeight(dist(rnd)),
                         innovation++);

    if (g_config.implicit_bias_links)
      genes.emplace_back(
          kBiasNodeId, kFirstOutput + out, ann::roundWeight(dist(rnd)), innovation++);

    if (g_config.recurrent_output_nodes) {
      Gene self_link(kFirstOutput + out,
                     kFirstOutput + out,
                     ann::roundWeight(dist(rnd)),
                     innovation++);
      self_link.recurrent = true;
      genes.push_back(self_link);
    }
  }

  if (g_config.use_lstm_nodes)
    for (float& w : lw)
      w = ann::roundWeight(dist(rnd));

  // return the next available innovation number
  return innovation;
}

void Genotype::mutate(atomic<Innovation>& next_innovation, bool weights_only) {
  std::default_random_engine rnd(core::randomSeed());

  mutateWeights(rnd);

  if (!weights_only) {
    mutateNewNodes(rnd, next_innovation);
    mutateNewLinks(rnd, next_innovation);
  }
}

void Genotype::inherit(const Genotype& parent1,
                       const Genotype& parent2,
                       float preference) {
  reset();

  const NodeId kHiddenFirst = 1 + g_inputs + g_outputs;

  std::default_random_engine rnd(core::randomSeed());
  std::bernoulli_distribution dist_parent(preference);
  bool use_parent1 = preference >= 0.5f;

  unordered_map<NodeId, NodeId> nodes_map1;
  unordered_map<NodeId, NodeId> nodes_map2;

  // map matching nodes from both parents
  auto map_matching_nodes = [&](NodeId node1, NodeId node2) {
    // fixed node IDs?
    if (node1 < kHiddenFirst || node2 < kHiddenFirst) {
      CHECK(node1 == node2);
      return node1;
    }

    auto it1 = nodes_map1.find(node1);
    auto it2 = nodes_map2.find(node2);

    // already mapped?
    if (it1 != nodes_map1.end()) {
      CHECK(it2 != nodes_map2.end());
      CHECK(it1->second == it2->second);
      return it1->second;
    }

    // allocate a new hidden node and update the maps
    NodeId new_node = nodes_count++;
    nodes_map1[node1] = new_node;
    nodes_map2[node2] = new_node;
    return new_node;
  };

  auto map_node = [&](NodeId node, unordered_map<NodeId, NodeId>& nodes_map) {
    if (node < kHiddenFirst)
      return node;

    NodeId& mapped_node = nodes_map[node];
    if (mapped_node == 0)
      mapped_node = nodes_count++;
    return mapped_node;
  };

  if (g_config.use_lstm_nodes)
    lw = dist_parent(rnd) ? parent1.lw : parent2.lw;

  // merge the genes from the parents
  auto g1 = parent1.genes.begin();
  auto g2 = parent2.genes.begin();
  while (g1 != parent1.genes.end() || g2 != parent2.genes.end()) {
    if (g1 != parent1.genes.end() && g2 != parent2.genes.end()) {
      if (g1->innovation == g2->innovation) {
        CHECK(g1->recurrent == g2->recurrent);
        Gene gene = dist_parent(rnd) ? *g1 : *g2;

        if (g_config.preserve_connectivity) {
          // make sure we don't mix disabled genes from a parent
          // w/o also carring the mutation which replaced it
          if (!gene.enabled && g1->enabled != g2->enabled) {
            if ((g1->enabled && use_parent1) || (g2->enabled && !use_parent1))
              gene.enabled = true;
          }
        }

        gene.in = map_matching_nodes(g1->in, g2->in);
        gene.out = map_matching_nodes(g1->out, g2->out);
        genes.push_back(gene);
        ++g1;
        ++g2;
      } else if (g1->innovation < g2->innovation) {
        if (use_parent1) {
          Gene gene = *g1;
          gene.in = map_node(gene.in, nodes_map1);
          gene.out = map_node(gene.out, nodes_map1);
          genes.push_back(gene);
        }
        ++g1;
      } else {
        if (!use_parent1) {
          Gene gene = *g2;
          gene.in = map_node(gene.in, nodes_map2);
          gene.out = map_node(gene.out, nodes_map2);
          genes.push_back(gene);
        }
        ++g2;
      }
    } else if (g1 != parent1.genes.end()) {
      assert(g2 == parent2.genes.end());
      if (use_parent1) {
        Gene gene = *g1;
        gene.in = map_node(gene.in, nodes_map1);
        gene.out = map_node(gene.out, nodes_map1);
        genes.push_back(gene);
      }
      ++g1;
    } else {
      assert(g1 == parent1.genes.end());
      assert(g2 != parent2.genes.end());
      if (!use_parent1) {
        Gene gene = *g2;
        gene.in = map_node(gene.in, nodes_map2);
        gene.out = map_node(gene.out, nodes_map2);
        genes.push_back(gene);
      }
      ++g2;
    }
  }
}

double Genotype::compatibility(const Genotype& ref) const {
  const auto& genes1 = genes;
  const auto& genes2 = ref.genes;

  double W = 0;
  size_t W_count = 0;
  size_t E_count = 0;
  size_t D_count = 0;

  auto g1 = genes1.begin();
  auto g2 = genes2.begin();
  while (g1 != genes1.end() || g2 != genes2.end()) {
    if (g1 != genes1.end() && g2 != genes2.end()) {
      if (g1->innovation == g2->innovation) {
        W += fabs(g1->weight - g2->weight);
        ++W_count;
        ++g1;
        ++g2;
      } else if (g1->innovation < g2->innovation) {
        ++D_count;
        ++g1;
      } else {
        ++D_count;
        ++g2;
      }
    } else if (g1 != genes1.end()) {
      assert(g2 == genes2.end());
      ++E_count;
      ++g1;
    } else {
      assert(g1 == genes1.end());
      assert(g2 != genes2.end());
      ++E_count;
      ++g2;
    }
  }

  constexpr double N = 1;  // same as the official NEAT implementation
  return (g_config.c1 * E_count) / N + (g_config.c2 * D_count) / N +
         g_config.c3 * (W / W_count);
}

unique_ptr<darwin::Brain> Genotype::grow() const {
  return make_unique<Brain>(this);
}

}  // namespace neat
