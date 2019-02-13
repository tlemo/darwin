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

#include "roulette_selection.h"

#include <core/exception.h>
#include <core/parallel_for_each.h>

using namespace selection;

namespace selection {

RouletteSelection::RouletteSelection(const core::PropertySet& config) {
  config_.copyFrom(config);
  if (config_.min_fitness < 0)
    throw core::Exception("Invalid configuration: min_fitness < 0");
  if (config_.elite_percentage < 0 || config_.elite_percentage > 100)
    throw core::Exception("Invalid configuration: elite_percentage");
}

void RouletteSelection::newPopulation(darwin::Population* population) {
  CHECK(population != nullptr);
  population_ = population;
}

// roulette wheel selection (aka fitness-proportionate selection)
// (supports negative fitness values too)
void RouletteSelection::createNextGeneration(GenerationFactory* next_generation) {
  const auto& ranking_index = population_->rankingIndex();
  const size_t population_size = ranking_index.size();
  CHECK(population_size > 0);

  constexpr float kMinFitness = 0.0f;
  vector<double> prefix_sum(population_size);
  double sum = 0;
  for (size_t i = 0; i < population_size; ++i) {
    const double fitness_value = population_->genotype(i)->fitness;
    sum += (fitness_value >= kMinFitness) ? fitness_value : 0.0f;
    prefix_sum[i] = sum;
  }

  const int elite_limit = max(1, int(population_size * config_.elite_percentage));

  pp::for_each(*next_generation, [&](int index, GenotypeFactory* genotype_factory) {
    const int old_genotype_index = int(ranking_index[index]);
    const auto old_genotype = population_->genotype(old_genotype_index);
    if (index < elite_limit && old_genotype->fitness >= config_.elite_min_fitness) {
      genotype_factory->replicate(old_genotype_index);
    } else {
      random_device rd;
      default_random_engine rnd(rd());

      auto selectParent = [&] {
        uniform_real_distribution<double> dist_sample(0, sum);
        const double sample = dist_sample(rnd);
        const auto interval = lower_bound(prefix_sum.begin(), prefix_sum.end(), sample);
        CHECK(interval != prefix_sum.end());
        return std::distance(prefix_sum.begin(), interval);
      };

      if (config_.mutation_only) {
        const int parent_index = selectParent();
        genotype_factory->replicate(parent_index);
        genotype_factory->mutate();
      } else {
        const int parent1 = selectParent();
        const int parent2 = selectParent();

        const float f1 = fmax(population_->genotype(parent1)->fitness, 0);
        const float f2 = fmax(population_->genotype(parent2)->fitness, 0);

        float preference = f1 / (f1 + f2);
        if (isnan(preference))
          preference = 0.5f;

        genotype_factory->crossover(parent1, parent2, preference);
        genotype_factory->mutate();
      }
    }
  });
}

}  // namespace selection
