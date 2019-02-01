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

#include "population.h"

#include <core/evolution.h>
#include <core/exception.h>
#include <core/logging.h>
#include <core/parallel_for_each.h>

#include <random>
#include <initializer_list>
#include <algorithm>
#include <iterator>
using namespace std;

namespace cgp {

Population::Population(const core::PropertySet& config, const darwin::Domain& domain) {
  config_.copyFrom(config);
  domain_ = &domain;
  
  // validate configuration
  if (config_.rows < 1)
    throw core::Exception("Invalid configuration: rows < 1");
  if (config_.columns < 1)
    throw core::Exception("Invalid configuration: columns < 1");
  if (config_.levels_back < 1)
    throw core::Exception("Invalid configuration: levels_back < 1");
  if (config_.elite_percentage < 0 || config_.elite_percentage > 100)
    throw core::Exception("Invalid configuration: elite_percentage");
    
  setupAvailableFunctions();
}

void Population::createPrimordialGeneration(int population_size) {
  CHECK(population_size > 0);

  core::log("Resetting evolution ...\n");

  darwin::StageScope stage("Create primordial generation");

  generation_ = 0;
  ranked_ = false;

  genotypes_.resize(population_size, Genotype(this));
  pp::for_each(genotypes_,
               [](int, Genotype& genotype) { genotype.createPrimordialSeed(); });

  core::log("Ready.\n");
}

void Population::rankGenotypes() {
  CHECK(!ranked_);

  // sort results by fitness (descending order)
  std::sort(genotypes_.begin(),
            genotypes_.end(),
            [](const Genotype& a, const Genotype& b) { return a.fitness > b.fitness; });

  // log best fitness values
  core::log("Fitness values: ");
  const size_t sample_size = min(size_t(16), genotypes_.size());
  for (size_t i = 0; i < sample_size; ++i) {
    core::log(" %.3f", genotypes_[i].fitness);
  }
  core::log(" ...\n");

  ranked_ = true;
}

void Population::createNextGeneration() {
  CHECK(ranked_);

  darwin::StageScope stage("Create next generation");

  ++generation_;

  // roulette wheel selection (aka fitness-proportionate selection)
  // (supports negative fitness values too)

  const size_t population_size = genotypes_.size();
  CHECK(population_size > 0);

  constexpr float kMinFitness = 0.0f;
  vector<double> prefix_sum(population_size);
  double sum = 0;
  for (size_t i = 0; i < population_size; ++i) {
    const double fitness_value = genotypes_[i].fitness;
    sum += (fitness_value >= kMinFitness) ? fitness_value : 0.0f;
    prefix_sum[i] = sum;
  }

  const int elite_limit = max(1, int(genotypes_.size() * config_.elite_percentage));

  vector<Genotype> next_generation(population_size, Genotype(this));
  pp::for_each(next_generation, [&](int index, Genotype& genotype) {
    if (index < elite_limit && genotypes_[index].fitness >= config_.elite_min_fitness) {
      genotype = genotypes_[index];
      genotype.genealogy = darwin::Genealogy("e", { index });
    } else {
      random_device rd;
      default_random_engine rnd(rd());
      uniform_real_distribution<double> dist_sample(0, sum);
      const double sample = dist_sample(rnd);
      const auto interval = lower_bound(prefix_sum.begin(), prefix_sum.end(), sample);
      CHECK(interval != prefix_sum.end());
      const int parent_index = std::distance(prefix_sum.begin(), interval);
      genotype = genotypes_[parent_index];
      genotype.mutate(config_.connection_mutation_chance,
                      config_.function_mutation_chance);
      genotype.genealogy = darwin::Genealogy("m", { parent_index });
    }
  });
  std::swap(genotypes_, next_generation);
  ranked_ = false;
}

void Population::setupAvailableFunctions() {
  CHECK(available_functions_.empty());

  auto addFunctions = [&](initializer_list<FunctionId> functions) {
    for (auto fn : functions) {
      available_functions_.push_back(fn);
    }
  };

  if (config_.fn_basic_constants) {
    core::log("CGP: adding basic constants...\n");
    addFunctions({
        FunctionId::ConstZero,
        FunctionId::ConstOne,
        FunctionId::ConstTwo,
    });
  }
  if (config_.fn_transcendental_constants) {
    core::log("CGP: adding transcendental constants...\n");
    addFunctions({
        FunctionId::ConstPi,
        FunctionId::ConstE,
    });
  }
  if (config_.fn_basic_arithmetic) {
    core::log("CGP: adding basic arithmetic...\n");
    addFunctions({
        FunctionId::Add,
        FunctionId::Subtract,
        FunctionId::Multiply,
        FunctionId::Divide,
        FunctionId::Negate,
    });
  }
  if (config_.fn_extra_arithmetic) {
    core::log("CGP: adding extra arithmetic functions...\n");
    addFunctions({
        FunctionId::Fmod,
        FunctionId::Reminder,
        FunctionId::Fdim,
        FunctionId::Ceil,
        FunctionId::Floor,
    });
  }
  if (config_.fn_common_math) {
    core::log("CGP: adding common math functions...\n");
    addFunctions({
        FunctionId::Abs,
        FunctionId::Average,
        FunctionId::Min,
        FunctionId::Max,
        FunctionId::Square,
    });
  }
  if (config_.fn_extra_math) {
    core::log("CGP: adding extra math functions...\n");
    addFunctions({
        FunctionId::Log,
        FunctionId::Log2,
        FunctionId::Sqrt,
        FunctionId::Power,
        FunctionId::Exp,
        FunctionId::Exp2,
    });
  }
  if (config_.fn_trigonometric) {
    core::log("CGP: adding trigonometric functions...\n");
    addFunctions({
        FunctionId::Sin,
        FunctionId::Cos,
        FunctionId::Tan,
        FunctionId::Asin,
        FunctionId::Acos,
        FunctionId::Atan,
    });
  }
  if (config_.fn_hyperbolic) {
    core::log("CGP: adding hyperbolic functions...\n");
    addFunctions({
        FunctionId::Sinh,
        FunctionId::Cosh,
        FunctionId::Tanh,
    });
  }
  if (config_.fn_ann_activation) {
    core::log("CGP: adding ANN activation functions...\n");
    addFunctions({
        FunctionId::AfnIdentity,
        FunctionId::AfnLogistic,
        FunctionId::AfnTanh,
        FunctionId::AfnReLU,
        FunctionId::AfnNeat,
    });
  }
  if (config_.fn_comparisons) {
    core::log("CGP: adding comparison functions...\n");
    addFunctions({
        FunctionId::CmpEq,
        FunctionId::CmpNe,
        FunctionId::CmpGt,
        FunctionId::CmpGe,
        FunctionId::CmpLt,
        FunctionId::CmpLe,
    });
  }
  if (config_.fn_logic_gates) {
    core::log("CGP: adding logic gate functions...\n");
    addFunctions({
        FunctionId::And,
        FunctionId::Or,
        FunctionId::Not,
        FunctionId::Xor,
    });
  }
  if (config_.fn_conditional) {
    core::log("CGP: adding conditional functions...\n");
    addFunctions({
        FunctionId::IfOrZero,
    });
  }
  
  core::log("CGP: %zu available functions\n\n", available_functions_.size());

  if (available_functions_.empty()) {
    throw core::Exception(
        "Invalid configuration: at least one function set must be selected");
  }
}

}  // namespace cgp
