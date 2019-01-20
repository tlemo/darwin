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

#include "population.h"
#include "neat.h"

#include <core/ann_activation_functions.h>
#include <core/evolution.h>
#include <core/logging.h>
#include <core/parallel_for_each.h>
#include <core/pp_utils.h>

#include <algorithm>
#include <limits>
using namespace std;

namespace neat {

void Population::createPrimordialGeneration(int population_size) {
  core::log("Resetting evolution ...\n");

  darwin::StageScope stage("Create primordial generation");

  generation_ = 0;
  next_innovation_ = 0;

  genotypes_.resize(population_size);
  order_.resize(population_size);

  resetOrder();

  pp::for_each(genotypes_, [this](int, Genotype& genotype) {
    auto innovation = genotype.createPrimordialSeed();
    pp::atomicMax(next_innovation_, innovation);
  });

  species_.clear();
  speciate();

  core::log("Ready.\n");
}

void Population::rankGenotypes() {
  CHECK(!ranked_);
  CHECK(order_.size() == genotypes_.size());

  // rank the genotypes by fitness (without touching the genotypes array)
  std::sort(order_.begin(), order_.end(), [&](size_t a, size_t b) {
    return genotypes_[a].fitness > genotypes_[b].fitness;
  });

  // log best fitness values
  core::log("Fitness values: ");
  const size_t sample_size = min(size_t(16), genotypes_.size());
  for (size_t i = 0; i < sample_size; ++i) {
    core::log(" %.3f", genotypes_[order_[i]].fitness);
  }
  core::log(" ...\n");

  ranked_ = true;
}

void Population::resetOrder() {
  CHECK(!order_.empty());
  CHECK(order_.size() == genotypes_.size());
  for (size_t i = 0; i < order_.size(); ++i)
    order_[i] = i;
  ranked_ = false;
}

void Population::assignSpecies(int index) {
  const auto& genotype = genotypes_[index];
  for (auto& species : species_) {
    if (genotype.compatibility(species.origin) < g_config.compatibility_threshold) {
      species.genotypes.push_back(index);
      return;
    }
  }

  // create a new species
  Species new_species;
  new_species.genotypes.push_back(index);
  new_species.origin = genotype;
  species_.push_back(new_species);
}

void Population::speciate() {
  darwin::StageScope stage("Speciate");

  for (int i = 0; i < genotypes_.size(); ++i)
    assignSpecies(i);

  // clean up extinct species
  auto removed =
      std::remove_if(species_.begin(), species_.end(), [](const Species& species) {
        return species.genotypes.empty();
      });
  species_.erase(removed, species_.end());
  CHECK(!species_.empty());
}

void Population::neatSelection() {
  CHECK(!species_.empty());

  // build the reverse mapping (direct index -> rank)
  // (this is needed for recording genealogy information, which
  // uses the ranked genotype indexes)
  vector<int> index_to_rank(order_.size());
  for (int i = 0; i < order_.size(); ++i)
    index_to_rank[order_[i]] = i;

  const size_t population_size = genotypes_.size();
  const float min_species_size = (float(population_size) / species_.size()) / 2.0f;

  const float worst_fitness = genotypes_[order_.back()].fitness;

  // NEAT requires non-negative fitness values, rebase now if needed
  // (adjust fitness values for all genotypes so worst fitness == 0)
  if (worst_fitness < 0) {
    for (auto& genotype : genotypes_)
      genotype.fitness -= worst_fitness;
  }

  atomic<size_t> total_size = 0;
  atomic<size_t> min_genotypes = population_size;
  atomic<size_t> max_genotypes = 0;

  // adjust fitness by species (explicit fitness sharing)
  pp::for_each(species_, [&](int, Species& species) {
    size_t size = species.genotypes.size();
    CHECK(size > 0 && size <= population_size);
    total_size += size;

    pp::atomicMax(max_genotypes, size);
    pp::atomicMin(min_genotypes, size);

    // adjust fitness
    //
    // NOTE: explicit fitness sharing with a twist: cap the
    //  min divisor to avoid extreme bias towards tiny species
    //
    for (int index : species.genotypes) {
      auto& fitness = genotypes_[index].fitness;
      CHECK(fitness >= 0);
      fitness /= max(float(size), min_species_size);
    }

    // rank the members of the species by fitness
    std::sort(species.genotypes.begin(), species.genotypes.end(), [this](int a, int b) {
      const auto& ga = genotypes_[a];
      const auto& gb = genotypes_[b];
      return ga.fitness > gb.fitness;
    });
  });

  // sanity check
  CHECK(total_size == population_size);

  // calculate overall fitness average
  double total_fitness = 0;
  for (int i = 0; i < population_size; ++i)
    total_fitness += genotypes_[i].fitness;
  double average_fitness = total_fitness / population_size;

  // print species stats
  //
  // TODO: distribution stats
  //
  core::log("Species: %zu, Distribution: avg=%.2f, min=%zu, max=%zu\n",
            species_.size(),
            genotypes_.size() / double(species_.size()),
            min_genotypes.load(),
            max_genotypes.load());

  // create the next generation
  vector<Genotype> next_generation(genotypes_.size());

  atomic<int> next_child = 0;
  atomic<int> extinct_species = 0;

  pp::for_each(species_, [&](int, Species& species) {
    std::random_device rd;
    std::default_random_engine rnd(rd());

    std::uniform_int_distribution<size_t> dist_parent_U(0, species.genotypes.size() - 1);

    std::discrete_distribution<size_t> dist_parent_D(
        species.genotypes.size(), 0, 1, [](double x) { return 1.1 - x; });

    auto dist_parent = [&](std::default_random_engine& rnd) {
      return g_config.uniform_parents_distribution ? dist_parent_U(rnd)
                                                   : dist_parent_D(rnd);
    };

    std::bernoulli_distribution dist_mutate_elite(g_config.elite_mutation_chance);

    double expected_offspring = 0;
    for (int i : species.genotypes)
      expected_offspring += genotypes_[i].fitness / average_fitness;
    expected_offspring = floor(expected_offspring);

    if (expected_offspring < g_config.min_species_size) {
      ++extinct_species;
    } else {
      for (int i = 0; i < expected_offspring; ++i) {
        int child_index = next_child++;
        CHECK(child_index < next_generation.size());
        auto& child = next_generation[child_index];

        float percentage = float(i) / species.genotypes.size();

        if (percentage < g_config.elite_percentage) {
          int parent = species.genotypes[i];
          child = genotypes_[parent];
          if (dist_mutate_elite(rnd)) {
            child.mutate(next_innovation_);
            child.genealogy = darwin::Genealogy("em", { index_to_rank[parent] });
          } else {
            child.genealogy = darwin::Genealogy("e", { index_to_rank[parent] });
          }
          ++child.age;
        } else {
          // pick two parents and produce the offspring
          int parent1 = species.genotypes[dist_parent(rnd)];
          int parent2 = species.genotypes[dist_parent(rnd)];

          const auto& g1 = genotypes_[parent1];
          const auto& g2 = genotypes_[parent2];

          float f1 = g1.fitness;
          float f2 = g2.fitness;

          float preference = f1 / (f1 + f2);
          if (isnan(preference))
            preference = 0.5f;

          child.inherit(g1, g2, preference);
          child.genealogy =
              darwin::Genealogy("c", { index_to_rank[parent1], index_to_rank[parent2] });
          child.mutate(next_innovation_);
        }
      }
    }
  });

  core::log("extinct species=%d\n", extinct_species.load());
  core::log("bonus interspecies=%d\n", int(next_generation.size()) - next_child);

  // fill in the rest with interspecies offsprings
  std::random_device rd;
  std::default_random_engine rnd(rd());
  std::uniform_int_distribution<int> dist_any_genome(0, int(genotypes_.size()) - 1);
  while (next_child < next_generation.size()) {
    int child_index = next_child++;
    CHECK(child_index < next_generation.size());
    auto& child = next_generation[child_index];

    int parent1 = dist_any_genome(rnd);
    int parent2 = dist_any_genome(rnd);

    const auto& g1 = genotypes_[parent1];
    const auto& g2 = genotypes_[parent2];

    float f1 = g1.fitness;
    float f2 = g2.fitness;

    float preference = f1 / (f1 + f2);
    if (isnan(preference))
      preference = 0.5f;

    child.inherit(g1, g2, preference);
    child.genealogy =
        darwin::Genealogy("i", { index_to_rank[parent1], index_to_rank[parent2] });
    child.mutate(next_innovation_);
  }

  std::swap(genotypes_, next_generation);

  // recreate species
  if (g_config.contiguous_species) {
    for (auto& species : species_)
      species.genotypes.clear();
  } else {
    species_.clear();
  }

  speciate();
}

void Population::classicSelection() {
  vector<Genotype> next_generation(genotypes_.size());

  atomic<size_t> elite_count = 0;
  atomic<size_t> babies_count = 0;
  atomic<size_t> mutate_count = 0;

  atomic<size_t> total_nodes_count = 0;
  atomic<size_t> total_genes_count = 0;
  atomic<size_t> max_nodes_count = 0;
  atomic<size_t> max_genes_count = 0;

  pp::for_each(next_generation, [&](int index, Genotype& genotype) {
    std::random_device rd;
    std::default_random_engine rnd(rd());
    std::uniform_int_distribution<int> dist_parent;
    std::uniform_real_distribution<double> dist_survive(0, 1);

    auto old_genotype = genotypes_[order_[index]];
    double time_left = (g_config.old_age - old_genotype.age) / double(g_config.old_age);

    bool viable = old_genotype.age < g_config.larva_age ||
                  old_genotype.fitness >= g_config.min_viable_fitness;

    // keep the elite population
    int elite_limit = max(2, int(genotypes_.size() * g_config.elite_percentage));
    if (index < elite_limit && old_genotype.fitness > g_config.elite_min_fitness) {
      // direct reproduction
      genotype = old_genotype;
      genotype.genealogy = darwin::Genealogy("e", { index });
      ++genotype.age;
      ++elite_count;
    } else if (index >= 2 && (!viable || dist_survive(rnd) > time_left)) {
      // pick two parents and produce the offspring
      int parent1 = dist_parent(rnd) % (index / 2);
      int parent2 = dist_parent(rnd) % (index / 2);

      const auto& g1 = genotypes_[order_[parent1]];
      const auto& g2 = genotypes_[order_[parent2]];

      float f1 = fmaxf(g1.fitness, 0);
      float f2 = fmaxf(g2.fitness, 0);

      float preference = f1 / (f1 + f2);
      if (isnan(preference))
        preference = 0.5f;

      genotype.inherit(g1, g2, preference);
      genotype.genealogy = darwin::Genealogy("c", { parent1, parent2 });
      genotype.mutate(next_innovation_);
      ++babies_count;
    } else {
      // last resort, mutate the old genotype
      genotype = old_genotype;
      genotype.genealogy = darwin::Genealogy("m", { index });
      genotype.mutate(next_innovation_, true);
      ++genotype.age;
      ++mutate_count;
    }

    total_nodes_count += genotype.nodes_count;
    total_genes_count += genotype.genes.size();
    pp::atomicMax(max_nodes_count, genotype.nodes_count);
    pp::atomicMax(max_genes_count, genotype.genes.size());
  });

  std::swap(genotypes_, next_generation);

  const double population_size = genotypes_.size();

  core::log("Next gen stats: %.2f%% elite, %.2f%% babies, %.2f%% mutate\n",
            (elite_count / population_size) * 100,
            (babies_count / population_size) * 100,
            (mutate_count / population_size) * 100);

  core::log("Next gen stats: %.2f avg nodes, %.2f avg genes\n",
            total_nodes_count / population_size,
            total_genes_count / population_size);

  core::log("Next gen stats: %zu max nodes, %zu max genes\n",
            max_nodes_count.load(),
            max_genes_count.load());
}

void Population::createNextGeneration() {
  CHECK(ranked_);

  darwin::StageScope stage("Create next generation");

  if (g_config.use_classic_selection) {
    classicSelection();
  } else {
    neatSelection();
  }

  ++generation_;

  resetOrder();
}

}  // namespace neat
