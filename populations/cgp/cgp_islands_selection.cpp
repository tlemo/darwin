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

using namespace selection;

namespace cgp {

CgpIslandsSelection::CgpIslandsSelection(const core::PropertySet& config) {
  config_.copyFrom(config);
  if (config_.island_size < 1)
    throw core::Exception("Invalid configuration: island_size < 1");
}

void CgpIslandsSelection::newPopulation(darwin::Population* population) {
  CHECK(population != nullptr);
  population_ = population;
}

void CgpIslandsSelection::createNextGeneration(GenerationFactory* next_generation) {
  // identify the parent for each island's next generation
  const int island_size = config_.island_size;
  const int population_size = int(population_->size());
  const int islands_count = (population_size + island_size - 1) / island_size;
  vector<int> island_parent(islands_count);
  for(int island = 0; island < islands_count; ++island) {
    const int island_base = island * island_size;
    int parent = 0;
    for (int i = 1; i < island_size && island_base + i < population_size; ++i) {
      if (population_->genotype(island_base + i)->fitness >=
          population_->genotype(island_base + parent)->fitness) {
        parent = i;
      }
    }
    island_parent[island] = island_base + parent;
  }

  // generate the next generation
  // (island parents are placed first within the island)
  pp::for_each(*next_generation, [&](int index, GenotypeFactory* genotype_factory) {
    const int island = index / island_size;
    const int parent = island_parent[island];
    CHECK(parent / island_size == island);
    genotype_factory->replicate(parent);
    if ((index % island_size) != 0) {
      genotype_factory->mutate();
    }
  });
}

}  // namespace cgp
