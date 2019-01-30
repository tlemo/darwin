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

#include "cgp.h"
#include "genotype.h"
#include "brain.h"
#include "population.h"

#include <random>
#include <limits>
using namespace std;

namespace cgp {

Genotype::Genotype(const Population* population) : population_(population) {}

unique_ptr<darwin::Brain> Genotype::grow() const {
  return make_unique<Brain>(this);
}

unique_ptr<darwin::Genotype> Genotype::clone() const {
  return make_unique<Genotype>(*this);
}

json Genotype::save() const {
  json json_obj;
  // TODO
  return json_obj;
}

void Genotype::load(const json& json_obj) {
  Genotype tmp_genotype(population_);
  // TODO
  std::swap(*this, tmp_genotype);
}

void Genotype::reset() {
  darwin::Genotype::reset();
  function_genes_.clear();
  output_genes_.clear();
}

void Genotype::createPrimordialSeed() {
  const auto& config = population_->config();
  CHECK(config.rows > 0);
  CHECK(config.columns > 0);
  
  function_genes_.resize(config.rows * config.columns);
  output_genes_.resize(population_->domain()->outputs());

  // randomize all connections and functions  
  mutate(1.0f, 1.0f);
}

void Genotype::mutate(float connection_mutation_chance, float function_mutation_chance) {
  const auto& config = population_->config();

  random_device rd;
  default_random_engine rnd(rd());
  bernoulli_distribution dist_mutate_connection(connection_mutation_chance);
  bernoulli_distribution dist_mutate_function(function_mutation_chance);

  // function genes
  uniform_int_distribution<int> dist_function_id(0, kFunctionCount - 1);
  for (int col = 0; col < config.columns; ++col) {
    const auto range = connectionRange(col + 1, config.levels_back);
    uniform_int_distribution<IndexType> dist_connection(range.first, range.second);
    for (int row = 0; row < config.rows; ++row) {
      auto& gene = function_genes_[row + col * config.rows];
      if (dist_mutate_function(rnd)) {
        gene.function = FunctionId(dist_function_id(rnd));
      }
      for (auto& connection : gene.connections) {
        if (dist_mutate_connection(rnd)) {
          connection = dist_connection(rnd);
        }
      }
    }
  }

  // output genes
  const auto output_layer = config.columns + 1;
  const auto output_levels_back =
      config.outputs_use_levels_back ? config.levels_back : output_layer;
  const auto range = connectionRange(output_layer, output_levels_back);
  uniform_int_distribution<IndexType> dist_connection(range.first, range.second);
  for (OutputGene& gene : output_genes_) {
    if (dist_mutate_connection(rnd)) {
      gene.connection = dist_connection(rnd);
    }
  }
}

pair<IndexType, IndexType> Genotype::connectionRange(int layer, int levels_back) const {
  const auto& config = population_->config();
  const size_t inputs_count = population_->domain()->inputs();
  CHECK(layer > 0 && layer <= config.columns + 1);
  CHECK(levels_back > 0);

  auto layerBaseIndex = [&](int layer) {
    return layer == 0 ? 0 : inputs_count + (layer - 1) * config.rows;
  };

  const int min_connection_layer = max(layer - levels_back, 0);
  const size_t min_index = layerBaseIndex(min_connection_layer);
  const size_t max_index = layerBaseIndex(layer) - 1;
  CHECK(max_index <= numeric_limits<IndexType>::max());
  return { IndexType(min_index), IndexType(max_index) };
}

}  // namespace cgp
