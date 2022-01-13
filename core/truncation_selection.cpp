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

#include "truncation_selection.h"

#include <core/exception.h>
#include <core/parallel_for_each.h>
#include <core/logging.h>
#include <core/random.h>

#include <algorithm>
#include <atomic>
#include <random>
using namespace std;

using namespace selection;

namespace selection {

TruncationSelection::TruncationSelection(const core::PropertySet& config) {
  config_.copyFrom(config);
  if (config_.elite_mutation_chance < 0 || config_.elite_mutation_chance > 1)
    throw core::Exception("Invalid configuration: elite_mutation_chance");
  if (config_.elite_percentage < 0 || config_.elite_percentage > 100)
    throw core::Exception("Invalid configuration: elite_percentage");
}

void TruncationSelection::newPopulation(darwin::Population* population) {
  CHECK(population != nullptr);
  population_ = population;
}

void TruncationSelection::createNextGeneration(GenerationFactory* next_generation) {
  atomic<size_t> elite_count = 0;
  atomic<size_t> mutate_only_count = 0;

  const auto& ranking_index = population_->rankingIndex();
  const int elite_limit = max(2, int(population_->size() * config_.elite_percentage));
  
  pp::for_each(*next_generation, [&](int index, GenotypeFactory* genotype_factory) {
    default_random_engine rnd(core::randomSeed());
    bernoulli_distribution dist_mutate_elite(config_.elite_mutation_chance);

    const int old_genotype_index = int(ranking_index[index]);
    auto old_genotype = population_->genotype(old_genotype_index);

    if (index < elite_limit && old_genotype->fitness >= config_.elite_min_fitness) {
      // direct reproduction
      genotype_factory->replicate(old_genotype_index);
      if (dist_mutate_elite(rnd)) {
        genotype_factory->mutate();
      }
      ++elite_count;
    } else if (index >= 2) {
      //  crossover (pick two parents and produce the offspring)
      uniform_int_distribution<int> dist_parent(0, index / 2);
      const int parent1 = int(ranking_index[dist_parent(rnd)]);
      const int parent2 = int(ranking_index[dist_parent(rnd)]);
      
      const float f1 = fmax(population_->genotype(parent1)->fitness, 0);
      const float f2 = fmax(population_->genotype(parent2)->fitness, 0);

      float preference = f1 / (f1 + f2);
      if (isnan(preference))
        preference = 0.5f;

      genotype_factory->crossover(parent1, parent2, preference);
      genotype_factory->mutate();
    } else {
      // last resort, mutate the old genotype
      genotype_factory->replicate(old_genotype_index);
      genotype_factory->mutate();
      ++mutate_only_count;
    }
  });

  const double population_size = population_->size();

  core::log("Truncation selection stats: %.2f%% elite, %.2f%% mutate_only\n",
            (elite_count / population_size) * 100,
            (mutate_only_count / population_size) * 100);
}

}  // namespace selection
