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

#include <core/exception.h>

namespace test_population {

Population::Population(const core::PropertySet& config, const darwin::Domain& domain) {
  config_.copyFrom(config);
  domain_ = &domain;
  
  // validate configuration
  if (config_.input_range < 0)
    throw core::Exception("Invalid configuration: input_range < 0");
  if (config_.output_range < 0)
    throw core::Exception("Invalid configuration: output_range < 0");
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

void Population::createPrimordialGeneration(int population_size) {
  CHECK(population_size > 0);
  generation_ = 0;
  genotypes_.resize(population_size, Genotype(this));
}

void Population::createNextGeneration() {
  CHECK(!genotypes_.empty());

  ++generation_;

  const int elite_limit = max(0, int(genotypes_.size() * config_.elite_percentage));
  for (int index = int(genotypes_.size()) - 1; index >= 0; --index) {
    Genotype& genotype = genotypes_[index];
    if (index < elite_limit && genotype.fitness >= config_.elite_min_fitness) {
      break;
    }
    genotype.reset();
  }
}

}  // namespace test_population
