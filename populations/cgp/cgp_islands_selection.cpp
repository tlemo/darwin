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

#include "cgp_islands_selection.h"

#include <core/exception.h>
#include <core/parallel_for_each.h>
#include <core/logging.h>

#include <algorithm>
using namespace std;

using namespace selection;

namespace cgp {

CgpIslandsSelection::CgpIslandsSelection(const core::PropertySet& config) {
  config_.copyFrom(config);
  if (config_.island_size < 1)
    throw core::Exception("Invalid configuration: island_size < 1");
  if (config_.protected_age < 0)
    throw core::Exception("Invalid configuration: protected_age < 0");
  if (config_.extinction_percentage < 0)
    throw core::Exception("Invalid configuration: extinction_percentage < 0");
  if (config_.extinction_percentage > 1)
    throw core::Exception("Invalid configuration: extinction_percentage > 1");
}

void CgpIslandsSelection::newPopulation(darwin::Population* population) {
  CHECK(population != nullptr);
  population_ = population;
  
  const size_t island_size = config_.island_size;
  const size_t islands_count = (population_->size() + island_size - 1) / island_size;
  islands_.clear();
  islands_.resize(islands_count);
}

void CgpIslandsSelection::createNextGeneration(GenerationFactory* next_generation) {
  // identify the candidates for each island's next generation parent
  const int island_size = config_.island_size;
  const int population_size = int(population_->size());
  const int islands_count = int(islands_.size());
  for (int island_index = 0; island_index < islands_count; ++island_index) {
    const int island_base = island_index * island_size;
    const int island_end = min(island_base + island_size, population_size);
    int parent = island_base;
    for (int i = parent + 1; i < island_end; ++i) {
      if (population_->genotype(i)->fitness >= population_->genotype(parent)->fitness) {
        parent = i;
      }
    }
    islands_[island_index].parent = parent;
    ++islands_[island_index].age;
  }

  // rank the islands by the parent (best) fitness, in ascending order
  // (so the lowest performing islands are at the beginning)
  vector<int> islands_ranking(islands_count);
  for (int i = 0; i < islands_count; ++i) {
    islands_ranking[i] = i;
  }
  std::sort(islands_ranking.begin(), islands_ranking.end(), [&](int a, int b) {
    const float a_fitness = population_->genotype(islands_[a].parent)->fitness;
    const float b_fitness = population_->genotype(islands_[b].parent)->fitness;
    return a_fitness < b_fitness;
  });

  // identify extinct islands
  const int extinct_limit = int(islands_count * config_.extinction_percentage);
  int actual_extinct = 0;
  for (int island_index = 0; island_index < extinct_limit; ++island_index) {
    auto& island = islands_[islands_ranking[island_index]];
    if (island.age >= config_.protected_age) {
      island.parent = kPrimordialSeed;
      island.age = 0;
      ++actual_extinct;
    }
  }
  core::log("extinct islands = %d (target = %d)\n", actual_extinct, extinct_limit);

  // generate the next generation
  // (island parents are placed first within the island)
  pp::for_each(*next_generation, [&](int index, GenotypeFactory* genotype_factory) {
    const int island_index = index / island_size;
    const int parent = islands_[island_index].parent;
    if (parent == kPrimordialSeed) {
      genotype_factory->createPrimordialSeed();
    } else {
      CHECK(parent / island_size == island_index);
      genotype_factory->replicate(parent);
      if ((index % island_size) != 0) {
        genotype_factory->mutate();
      }
    }
  });
}

}  // namespace cgp
