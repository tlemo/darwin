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

#include <algorithm>
#include <atomic>
#include <random>
#include <vector>
#include <memory>
using namespace std;

namespace classic {

template <class GENOTYPE>
class Population : public darwin::Population {
  class GenotypeFactory : public selection::GenotypeFactory {
   public:
    void init(Population* population, GENOTYPE* genotype) {
      population_ = population;
      genotype_ = genotype;
    }

    void createPrimordialSeed() override {
      genotype_->createPrimordialSeed();
      genotype_->genealogy = darwin::Genealogy("p", {});
    }

    void replicate(int parent_index) override {
      *genotype_ = population_->genotypes_[parent_index];
      genotype_->genealogy = darwin::Genealogy("r", { parent_index });
    }

    void crossover(int parent1, int parent2, float preference) override {
      genotype_->inherit(
          population_->genotypes_[parent1], population_->genotypes_[parent2], preference);
      genotype_->genealogy = darwin::Genealogy("c", { parent1, parent2 });
    }

    void mutate() override {
      genotype_->mutate();
      genotype_->genealogy.genetic_operator += "m";
    }

   private:
    Population* population_ = nullptr;
    GENOTYPE* genotype_ = nullptr;
  };

  class GenerationFactory : public selection::GenerationFactory {
   public:
    GenerationFactory(Population* population, vector<GENOTYPE>& next_generation) {
      factories_.resize(next_generation.size());
      for (size_t i = 0; i < factories_.size(); ++i) {
        factories_[i].init(population, &next_generation[i]);
      }
    }

    size_t size() const override { return factories_.size(); }
    GenotypeFactory* operator[](size_t index) override { return &factories_[index]; }

   private:
    vector<GenotypeFactory> factories_;
  };

 public:
  Population() {
    switch (g_config.selection_algorithm.tag()) {
      case SelectionAlgorithmType::RouletteWheel:
        selection_algorithm_ = make_unique<selection::RouletteSelection>(
            g_config.selection_algorithm.roulette_wheel);
        break;
      case SelectionAlgorithmType::CgpIslands:
        selection_algorithm_ = make_unique<selection::CgpIslandsSelection>(
            g_config.selection_algorithm.cgp_islands);
        break;
      case SelectionAlgorithmType::Truncation:
        selection_algorithm_ = make_unique<selection::TruncationSelection>(
            g_config.selection_algorithm.truncation);
        break;
      default:
        FATAL("Unexpected selection algorithm type");
    }
  }

  size_t size() const override { return genotypes_.size(); }

  int generation() const override { return generation_; }

  GENOTYPE* genotype(size_t index) override { return &genotypes_[index]; }

  const GENOTYPE* genotype(size_t index) const override { return &genotypes_[index]; }

  void createPrimordialGeneration(int population_size) override {
    core::log("Resetting evolution ...\n");

    darwin::StageScope stage("Create primordial generation");

    generation_ = 0;

    genotypes_.resize(population_size);
    pp::for_each(genotypes_,
                 [](int, GENOTYPE& genotype) { genotype.createPrimordialSeed(); });

    selection_algorithm_->newPopulation(this);
    core::log("Ready.\n");
  }

  void createNextGeneration() override {
    darwin::StageScope stage("Create next generation");

    ++generation_;
    vector<GENOTYPE> next_generation(genotypes_.size());
    GenerationFactory generation_factory(this, next_generation);
    selection_algorithm_->createNextGeneration(&generation_factory);
    std::swap(genotypes_, next_generation);
  }

  vector<size_t> rankingIndex() const {
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

 private:
  vector<GENOTYPE> genotypes_;
  int generation_ = 0;
  
  unique_ptr<selection::SelectionAlgorithm> selection_algorithm_;
};

}  // namespace classic
