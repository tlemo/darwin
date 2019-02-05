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

#include "cgp.h"
#include "genotype.h"

#include <core/darwin.h>
#include <core/properties.h>

#include <vector>
using namespace std;

namespace cgp {

class Population : public darwin::Population {
 public:
  Population(const core::PropertySet& config, const darwin::Domain& domain);

  size_t size() const override { return genotypes_.size(); }
  int generation() const override { return generation_; }

  Genotype* genotype(size_t index) override { return &genotypes_[index]; }
  const Genotype* genotype(size_t index) const override { return &genotypes_[index]; }

  vector<size_t> rankingIndex() const override;
  void createPrimordialGeneration(int population_size) override;
  void createNextGeneration() override;

  const Config& config() const { return config_; }
  const darwin::Domain* domain() const { return domain_; }
  
  const vector<FunctionId>& availableFunctions() const { return available_functions_; }
  
 private:
  void setupAvailableFunctions();

 private:
  Config config_;
  const darwin::Domain* domain_ = nullptr;
  
  vector<Genotype> genotypes_;
  int generation_ = 0;
  
  vector<FunctionId> available_functions_;
};

}  // namespace cgp
