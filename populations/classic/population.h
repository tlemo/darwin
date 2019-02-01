// Copyright 2018 The Darwin Neuroevolution Framework Authors.
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

#pragma once

#include "classic.h"

#include <core/ann_activation_functions.h>
#include <core/utils.h>
#include <core/darwin.h>
#include <core/evolution.h>
#include <core/logging.h>
#include <core/parallel_for_each.h>

#include <assert.h>
#include <algorithm>
#include <atomic>
#include <random>
#include <vector>
using namespace std;

namespace classic {

template <class GENOTYPE>
class Population : public darwin::Population {
 public:
  size_t size() const override { return genotypes_.size(); }

  int generation() const override { return generation_; }

  GENOTYPE* genotype(size_t index) override { return &genotypes_[index]; }

  const GENOTYPE* genotype(size_t index) const override { return &genotypes_[index]; }

  void createPrimordialGeneration(int population_size) override {
    core::log("Resetting evolution ...\n");

    darwin::StageScope stage("Create primordial generation");

    generation_ = 0;
    ranked_ = false;

    genotypes_.resize(population_size);
    pp::for_each(genotypes_,
                 [](int, GENOTYPE& genotype) { genotype.createPrimordialSeed(); });

    core::log("Ready.\n");
  }

  void createNextGeneration() override {
    CHECK(ranked_);

    darwin::StageScope stage("Create next generation");

    ++generation_;

    vector<GENOTYPE> next_generation(genotypes_.size());

    atomic<size_t> elite_count = 0;
    atomic<size_t> babies_count = 0;
    atomic<size_t> mutate_count = 0;

    pp::for_each(next_generation, [&](int index, GENOTYPE& genotype) {
      std::random_device rd;
      std::default_random_engine rnd(rd());
      std::uniform_int_distribution<int> dist_parent;
      std::uniform_real_distribution<double> dist_survive(0, 1);
      std::bernoulli_distribution dist_mutate_elite(g_config.elite_mutation_chance);
  
      auto old_genotype = genotypes_[index];
      const double old_age = g_config.old_age;
      const double time_left = old_age > 0 ? (old_age - old_genotype.age) / old_age : 0;

      const bool viable = old_genotype.age < g_config.larva_age ||
                    old_genotype.fitness >= g_config.min_viable_fitness;

      // keep the elite population
      const int elite_limit = max(2, int(genotypes_.size() * g_config.elite_percentage));
      if (index < elite_limit && old_genotype.fitness >= g_config.elite_min_fitness) {
        // direct reproduction
        genotype = old_genotype;
        if (dist_mutate_elite(rnd)) {
          genotype.mutate();
          genotype.genealogy = darwin::Genealogy("em", { index });
        } else {
          genotype.genealogy = darwin::Genealogy("e", { index });
        }
        ++genotype.age;
        ++elite_count;
      } else if (index >= 2 && (!viable || dist_survive(rnd) > time_left)) {
        // pick two parents and produce the offspring
        int parent1 = dist_parent(rnd) % (index / 2);
        int parent2 = dist_parent(rnd) % (index / 2);

        const auto& g1 = genotypes_[parent1];
        const auto& g2 = genotypes_[parent2];

        float f1 = fmaxf(g1.fitness, 0);
        float f2 = fmaxf(g2.fitness, 0);

        float preference = f1 / (f1 + f2);
        if (isnan(preference))
          preference = 0.5f;

        genotype.inherit(g1, g2, preference);
        genotype.genealogy = darwin::Genealogy("c", { parent1, parent2 });
        genotype.mutate();
        ++babies_count;
      } else {
        // last resort, mutate the old genotype
        genotype = old_genotype;
        genotype.genealogy = darwin::Genealogy("m", { index });
        genotype.mutate();
        ++genotype.age;
        ++mutate_count;
      }
    });

    std::swap(genotypes_, next_generation);

    const double population_size = genotypes_.size();

    core::log("Next gen stats: %.2f%% elite, %.2f%% babies, %.2f%% mutate\n",
              (elite_count / population_size) * 100,
              (babies_count / population_size) * 100,
              (mutate_count / population_size) * 100);

    ranked_ = false;
  }

  void rankGenotypes() override {
    CHECK(!ranked_);

    // sort results by fitness (descending order)
    std::sort(genotypes_.begin(),
              genotypes_.end(),
              [](const GENOTYPE& a, const GENOTYPE& b) { return a.fitness > b.fitness; });

    // log best fitness values
    core::log("Fitness values: ");
    const size_t sample_size = min(size_t(16), genotypes_.size());
    for (size_t i = 0; i < sample_size; ++i) {
      core::log(" %.3f", genotypes_[i].fitness);
    }
    core::log(" ...\n");

    ranked_ = true;
  }

 private:
  vector<GENOTYPE> genotypes_;
  int generation_ = 0;
  bool ranked_ = false;
};

}  // namespace classic
