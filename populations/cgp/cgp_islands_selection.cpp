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

  const int island_size = config_.island_size;
  const int islands_count = (population_->size() + island_size - 1) / island_size;
  island_parent_.resize(islands_count);
  for (int& parent : island_parent_) {
    parent = kNoParent;
  }
}

void CgpIslandsSelection::createNextGeneration(GenerationFactory* next_generation) {
  // identify each island's next generation parent
  for (size_t island = 0; island < island_parent_.size(); ++island) {
    const int island_base = island * config_.island_size;
    const int prev_parent = island_parent_[island];
    int parent = prev_parent;
    for (int i = 0; i < config_.island_size; ++i) {
      if (i != prev_parent) {
        if (population_->genotype(island_base + i)->fitness >=
            population_->genotype(island_base + parent)->fitness) {
          parent = i;
        }
      }
    }
    island_parent_[island] = parent;
  }

  // generate the next generation
  pp::for_each(*next_generation, [&](int index, GenotypeFactory* genotype_factory) {
    const int island = index / config_.island_size;
    const int island_base = island * config_.island_size;
    const int island_parent = island_base + island_parent_[island];
    genotype_factory->replicate(island_parent);
    if (index != island_parent) {
      genotype_factory->mutate();
    }
  });
}

}  // namespace cgp
