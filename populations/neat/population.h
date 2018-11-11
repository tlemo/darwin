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

#include "genotype.h"

#include <core/darwin.h>

namespace neat {

struct Species {
  vector<int> genotypes;
  Genotype origin;
};

class Population : public darwin::Population {
 public:
  size_t size() const override { return genotypes_.size(); }

  int generation() const override { return generation_; }

  Genotype* genotype(size_t index) override { return &genotypes_[order_[index]]; }

  const Genotype* genotype(size_t index) const override {
    return &genotypes_[order_[index]];
  }

  void createPrimordialGeneration(int population_size) override;

  void rankGenotypes() override;

  void createNextGeneration() override;

 private:
  void classicSelection();
  void neatSelection();
  void resetOrder();

  // separate the genomes into species
  void speciate();
  void assignSpecies(int index);

 protected:
  vector<Genotype> genotypes_;
  vector<Species> species_;
  atomic<Innovation> next_innovation_ = 0;

  int generation_ = 0;

  bool ranked_ = false;
  vector<size_t> order_;
};

class Factory : public darwin::PopulationFactory {
  unique_ptr<darwin::Population> create(const core::PropertySet& config,
                                        const darwin::Domain& domain) override;

  unique_ptr<core::PropertySet> defaultConfig(darwin::ComplexityHint hint) const override;
};

}  // namespace neat
