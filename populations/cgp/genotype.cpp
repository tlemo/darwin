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

#include <core/format.h>
#include <core/random.h>

#include <string>
#include <random>
#include <limits>
#include <algorithm>
using namespace std;

namespace cgp {

Genotype::Genotype(const Population* population) : population_(population) {}

unique_ptr<darwin::Brain> Genotype::grow() const {
  return make_unique<Brain>(this);
}

unique_ptr<darwin::Genotype> Genotype::clone() const {
  return make_unique<Genotype>(*this);
}

bool operator==(const FunctionGene& a, const FunctionGene& b) {
  return a.function == b.function && a.connections == b.connections;
}

bool operator==(const OutputGene& a, const OutputGene& b) {
  return a.connection == b.connection;
}

bool operator==(const Genotype& a, const Genotype& b) {
  return a.function_genes_ == b.function_genes_ &&
         a.output_genes_ == b.output_genes_ &&
         a.constants_genes_ == b.constants_genes_;
}

void to_json(json& json_obj, const FunctionGene& gene) {
  const string function_name = gene.function >= 0
                                   ? core::toString(gene.function)
                                   : core::format("const_%d", -gene.function);
  json_obj["fn"] = function_name;
  json_obj["c"] = gene.connections;
  json_obj["a"] = gene.function >= 0 ? kFunctionDef[gene.function].arity : 0;
}

void from_json(const json& json_obj, FunctionGene& gene) {
  constexpr char kPrefix[] = "const_";
  constexpr size_t kPrefixLen = std::size(kPrefix) - 1;
  const string function_name = json_obj.at("fn");
  if (function_name.compare(0, kPrefixLen, kPrefix) == 0) {
    const string const_index = function_name.substr(kPrefixLen);
    gene.function = FunctionId(-core::fromString<int>(const_index));
  } else {
    gene.function = core::fromString<FunctionId>(function_name);
  }
  CHECK(gene.function < FunctionId::LastEntry);
  const int arity = gene.function >= 0 ? kFunctionDef[gene.function].arity : 0;
  CHECK(json_obj.at("a") == arity);
  gene.connections = json_obj.at("c");
}

void to_json(json& json_obj, const OutputGene& gene) {
  json_obj["c"] = gene.connection;
}

void from_json(const json& json_obj, OutputGene& gene) {
  gene.connection = json_obj.at("c");
}

json Genotype::save() const {
  json json_obj;
  
  // shared values
  //
  // TODO: save these only once per population
  //
  json_obj["inputs"] = population_->domain()->inputs();
  json_obj["outputs"] = population_->domain()->outputs();
  json_obj["rows"] = population_->config().rows;
  json_obj["columns"] = population_->config().columns;
  
  // genotype encoding
  json_obj["function_genes"] = function_genes_;
  json_obj["output_genes"] = output_genes_;
  json_obj["constants_genes"] = constants_genes_;
  
  return json_obj;
}

void Genotype::load(const json& json_obj) {
  // check inputs & outputs count
  const size_t inputs = json_obj.at("inputs");
  const size_t outputs = json_obj.at("outputs");
  if (inputs != population_->domain()->inputs())
    throw core::Exception("Can't load genotype, mismatched inputs count");
  if (outputs != population_->domain()->outputs())
    throw core::Exception("Can't load genotype, mismatched outputs count");

  // check rows & columns count
  const int rows = json_obj.at("rows");
  const int columns = json_obj.at("columns");
  if (rows != population_->config().rows)
    throw core::Exception("Can't load genotype, mismatched rows count");
  if (columns != population_->config().columns)
    throw core::Exception("Can't load genotype, mismatched columns count");

  // load the genotype
  Genotype tmp_genotype(population_);
  tmp_genotype.function_genes_ =
      json_obj.at("function_genes").get<vector<FunctionGene>>();
  tmp_genotype.output_genes_ = json_obj.at("output_genes").get<vector<OutputGene>>();
  tmp_genotype.constants_genes_ = json_obj.at("constants_genes").get<vector<float>>();
  std::swap(*this, tmp_genotype);
}

void Genotype::reset() {
  darwin::Genotype::reset();
  function_genes_.clear();
  output_genes_.clear();
  constants_genes_.clear();
}

template <class PRED>
void Genotype::mutationHelper(PRED& predicates) {
  const auto& config = population_->config();

  const auto& available_functions = population_->availableFunctions();
  CHECK(!available_functions.empty());
  
  CHECK(!function_genes_.empty());
  CHECK(!output_genes_.empty());
  CHECK(!constants_genes_.empty());

  // function genes
  //  positive values: regular functions
  //  negative values: evolvable constants
  const int evolvable_constants_base = -int(constants_genes_.size());
  uniform_int_distribution<int> dist_function(evolvable_constants_base,
                                              int(available_functions.size()) - 1);
  for (int col = 0; col < config.columns; ++col) {
    const auto range = connectionRange(col + 1, config.levels_back);
    uniform_int_distribution<IndexType> dist_connection(range.first, range.second);
    for (int row = 0; row < config.rows; ++row) {
      auto& gene = function_genes_[row + col * config.rows];
      if (predicates.mutateFunction()) {
        const int index = dist_function(predicates.rnd);
        gene.function = index >= 0 ? available_functions[index] : FunctionId(index);
      }
      for (auto& connection : gene.connections) {
        if (predicates.mutateConnection()) {
          connection = dist_connection(predicates.rnd);
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
    if (predicates.mutateOutput()) {
      gene.connection = dist_connection(predicates.rnd);
    }
  }

  // evolvable constants
  for (float& value : constants_genes_) {
    if (predicates.mutateConstant()) {
      const float resolution = config.evolvable_constants_resolution;
      normal_distribution<float> dist_value(value, config.evolvable_constants_std_dev);
      value = int(dist_value(predicates.rnd) / resolution) * resolution;
    }
  }
}

void Genotype::createPrimordialSeed() {
  const auto& config = population_->config();
  CHECK(config.rows > 0);
  CHECK(config.columns > 0);

  function_genes_.resize(config.rows * config.columns);
  output_genes_.resize(population_->domain()->outputs());
  constants_genes_.resize(config.evolvable_constants_count);

  struct Predicates {
    default_random_engine rnd{ core::randomSeed() };
    bool mutateConnection() { return true; }
    bool mutateFunction() { return true; }
    bool mutateOutput() { return true; }
    bool mutateConstant() { return false; }
  } predicates;

  // evolvable constants
  const float resolution = config.evolvable_constants_resolution;
  uniform_real_distribution<float> dist_const_value(-config.evolvable_constants_range,
                                                    +config.evolvable_constants_range);
  for (float& value : constants_genes_) {
    value = int(dist_const_value(predicates.rnd) / resolution) * resolution;
  }

  // mutate everything else
  mutationHelper(predicates);
}

void Genotype::probabilisticMutation(const ProbabilisticMutation& config) {
  struct Predicates {
    default_random_engine rnd{ core::randomSeed() };
    bernoulli_distribution dist_mutate_connection;
    bernoulli_distribution dist_mutate_function;
    bernoulli_distribution dist_mutate_output;
    bernoulli_distribution dist_mutate_constant;

    Predicates(const ProbabilisticMutation& config)
        : dist_mutate_connection(config.connection_mutation_chance),
          dist_mutate_function(config.function_mutation_chance),
          dist_mutate_output(config.output_mutation_chance),
          dist_mutate_constant(config.constant_mutation_chance) {}

    bool mutateConnection() { return dist_mutate_connection(rnd); }
    bool mutateFunction() { return dist_mutate_function(rnd); }
    bool mutateOutput() { return dist_mutate_output(rnd); }
    bool mutateConstant() { return dist_mutate_constant(rnd); }
  } predicates(config);

  mutationHelper(predicates);
}

void Genotype::fixedCountMutation(const FixedCountMutation& config) {
  // calculate total number of genes in this genotype
  const size_t function_genes_count = function_genes_.size();
  const size_t connection_genes_count = function_genes_.size() * kMaxFunctionArity;
  const size_t output_genes_count = output_genes_.size();
  const size_t constant_genes_count = constants_genes_.size();
  const size_t total_genes_count = function_genes_count + connection_genes_count +
                                   output_genes_count + constant_genes_count;

  struct Predicates {
    default_random_engine rnd{ core::randomSeed() };
    double remaining_genes;
    double remaining_mutations;

    Predicates(size_t total_genes_count, const FixedCountMutation& config) {
      remaining_genes = double(total_genes_count);
      remaining_mutations = min(double(config.mutation_count), remaining_genes);
    }

    ~Predicates() {
      CHECK(remaining_genes == 0);
      CHECK(remaining_mutations == 0);
    }

    bool mutateGene() {
      bernoulli_distribution dist_mutate(remaining_mutations / remaining_genes);
      --remaining_genes;
      if (dist_mutate(rnd)) {
        --remaining_mutations;
        return true;
      }
      return false;
    }

    bool mutateConnection() { return mutateGene(); }
    bool mutateFunction() { return mutateGene(); }
    bool mutateOutput() { return mutateGene(); }
    bool mutateConstant() { return mutateGene(); }
  } predicates(total_genes_count, config);
  
  mutationHelper(predicates);
}

template <class T, class RND>
static vector<T> singlePointCrossoverHelper(const vector<T>& a,
                                            const vector<T>& b,
                                            RND& rnd) {
  CHECK(a.size() == b.size());

  vector<T> c(a.size());
  uniform_int_distribution<size_t> dist_split_point(0, c.size() - 1);
  bernoulli_distribution dist_coin;

  size_t split_point = dist_split_point(rnd);
  if (dist_coin(rnd)) {
    auto it = std::copy(a.begin(), a.begin() + split_point, c.begin());
    std::copy(b.begin() + split_point, b.end(), it);
  } else {
    auto it = std::copy(b.begin(), b.begin() + split_point, c.begin());
    std::copy(a.begin() + split_point, a.end(), it);
  }

  return a;
}

void Genotype::inherit(const Genotype& parent1,
                       const Genotype& parent2,
                       float /*preference*/) {
  default_random_engine rnd(core::randomSeed());

  function_genes_ =
      singlePointCrossoverHelper(parent1.function_genes_, parent2.function_genes_, rnd);
  output_genes_ =
      singlePointCrossoverHelper(parent1.output_genes_, parent2.output_genes_, rnd);
  constants_genes_ =
      singlePointCrossoverHelper(parent1.constants_genes_, parent2.constants_genes_, rnd);
}

float Genotype::getEvolvableConstant(int function_id) const {
  const int evolvable_constants_base = -int(constants_genes_.size());
  CHECK(function_id >= evolvable_constants_base && function_id < 0);
  return constants_genes_[function_id - evolvable_constants_base];
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
