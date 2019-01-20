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
#include "cgp.h"

#include <core/evolution.h>
#include <core/logging.h>
#include <core/parallel_for_each.h>

namespace cgp {

void Population::createPrimordialGeneration(int population_size) {
  // TODO
  generation_ = 0;
  ranked_ = false;
  genotypes_.resize(population_size);
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
  // TODO
  CHECK(ranked_);
  ++generation_;
  ranked_ = false;
}

}  // namespace cgp
