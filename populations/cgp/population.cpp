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
  if (config_.levels_back > config_.columns)
    throw core::Exception("Invalid configuration: levels_back > columns");
  if (config_.elite_percentage < 0 || config_.elite_percentage > 100)
    throw core::Exception("Invalid configuration: elite_percentage");
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
  CHECK(!genotypes_.empty());

  // TODO (pp too)
  const int elite_limit = max(1, int(genotypes_.size() * config_.elite_percentage));
  const int parents_count = int(genotypes_.size()) / 5;
  
  random_device rd;
  default_random_engine rnd(rd());
  uniform_int_distribution<int> dist_parent(0, parents_count - 1);
  
  for (int index = int(genotypes_.size()) - 1; index >= 0; --index) {
    Genotype& genotype = genotypes_[index];
    if (index < elite_limit && genotype.fitness >= config_.elite_min_fitness) {
      break;
    }
    genotype = genotypes_[dist_parent(rnd)];
    genotype.mutate(config_.connection_mutation_chance, config_.function_mutation_chance);
  }

  ++generation_;
  ranked_ = false;
}

}  // namespace cgp
