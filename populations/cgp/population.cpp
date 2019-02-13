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

class Population::GenotypeFactory : public selection::GenotypeFactory {
 public:
  void init(Population* population, Genotype* genotype) {
    population_ = population;
    genotype_ = genotype;
  }
  
  void createPrimordialSeed() override {
    genotype_->createPrimordialSeed();
    genotype_->genealogy = darwin::Genealogy("p", {});
  }

  void replicate(int parent_index) override {
    *genotype_ = population_->genotypes_[parent_index];
    genotype_->genealogy = darwin::Genealogy("r", { parent_index });
  }

  void crossover(int parent1, int parent2, float preference) override {
    genotype_->inherit(
        population_->genotypes_[parent1], population_->genotypes_[parent2], preference);
    genotype_->genealogy = darwin::Genealogy("c", { parent1, parent2 });
  }

  void mutate() override {
    const auto& config = population_->config_;
    switch (config.mutation_strategy.tag()) {
      case MutationStrategy::FixedCount:
        genotype_->fixedCountMutation(config.mutation_strategy.fixed_count);
        break;
      case MutationStrategy::Probabilistic:
        genotype_->probabilisticMutation(config.mutation_strategy.probabilistic);
        break;
      default:
        FATAL("Unexpected mutation strategy");
    }
    genotype_->genealogy.genetic_operator += "m";
  }

 private:
  Population* population_ = nullptr;
  Genotype* genotype_ = nullptr;
};

class Population::GenerationFactory : public selection::GenerationFactory {
 public:
  GenerationFactory(Population* population, vector<Genotype>& next_generation) {
    factories_.resize(next_generation.size());
    for (size_t i = 0; i < factories_.size(); ++i) {
      factories_[i].init(population, &next_generation[i]);
    }
  }

  size_t size() const override { return factories_.size(); }
  GenotypeFactory* operator[](size_t index) override { return &factories_[index]; }

 private:
  vector<GenotypeFactory> factories_;
};

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
  if (config_.mutation_strategy.fixed_count.mutation_count < 0)
    throw core::Exception(
        "Invalid configuration: fixed_mutation_count.mutation_count < 0");

  setupAvailableFunctions();

  switch (config_.selection_algorithm.tag()) {
    case SelectionAlgorithmType::RouletteWheel:
      selection_algorithm_ = make_unique<selection::RouletteSelection>(
          config_.selection_algorithm.roulette_wheel);
      break;
    case SelectionAlgorithmType::CgpIslands:
      selection_algorithm_ = make_unique<selection::CgpIslandsSelection>(
          config_.selection_algorithm.cgp_islands);
      break;
    case SelectionAlgorithmType::Truncation:
      selection_algorithm_ = make_unique<selection::TruncationSelection>(
          config_.selection_algorithm.truncation);
      break;
    default:
      FATAL("Unexpected selection algorithm type");
  }
}

void Population::createPrimordialGeneration(int population_size) {
  CHECK(population_size > 0);

  core::log("Resetting evolution ...\n");

  darwin::StageScope stage("Create primordial generation");

  generation_ = 0;

  genotypes_.resize(population_size, Genotype(this));
  pp::for_each(genotypes_,
               [](int, Genotype& genotype) { genotype.createPrimordialSeed(); });

  selection_algorithm_->newPopulation(this);
  core::log("Ready.\n");
}

vector<size_t> Population::rankingIndex() const {
  vector<size_t> ranking_index(genotypes_.size());
  for (size_t i = 0; i < ranking_index.size(); ++i) {
    ranking_index[i] = i;
  }
  // sort results by fitness (descending order)
  std::sort(ranking_index.begin(), ranking_index.end(), [&](size_t a, size_t b) {
    return genotypes_[a].fitness > genotypes_[b].fitness;
  });
  return ranking_index;
}

void Population::createNextGeneration() {
  darwin::StageScope stage("Create next generation");

  ++generation_;
  vector<Genotype> next_generation(genotypes_.size(), Genotype(this));
  GenerationFactory generation_factory(this, next_generation);
  selection_algorithm_->createNextGeneration(&generation_factory);
  std::swap(genotypes_, next_generation);
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
  if (config_.fn_stateful) {
    core::log("CGP: adding stateful functions...\n");
    addFunctions({
        FunctionId::Velocity,
        FunctionId::HighWatermark,
        FunctionId::LowWatermark,
        FunctionId::MemoryCell,
        FunctionId::SoftMemoryCell,
        FunctionId::TimeDelay,
    });
  }
  
  core::log("CGP: %zu available functions\n\n", available_functions_.size());

  if (available_functions_.empty()) {
    throw core::Exception(
        "Invalid configuration: at least one function set must be selected");
  }
}

}  // namespace cgp
