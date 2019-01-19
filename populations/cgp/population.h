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

#pragma once

#include "genotype.h"

#include <core/darwin.h>

#include <memory>
#include <vector>
using namespace std;

namespace cgp {

class Population : public darwin::Population {
 public:
  size_t size() const override { return genotypes_.size(); }
  int generation() const override { return generation_; }

  Genotype* genotype(size_t index) override { return &genotypes_[index]; }
  const Genotype* genotype(size_t index) const override { return &genotypes_[index]; }

  void rankGenotypes() override;
  void createPrimordialGeneration(int population_size) override;
  void createNextGeneration() override;

 private:
  vector<Genotype> genotypes_;
  int generation_ = 0;
  bool ranked_ = false;
};

class Factory : public darwin::PopulationFactory {
  unique_ptr<darwin::Population> create(const core::PropertySet& config,
                                        const darwin::Domain& domain) override;

  unique_ptr<core::PropertySet> defaultConfig(darwin::ComplexityHint hint) const override;
};

}  // namespace cgp
