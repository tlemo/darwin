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

#include "selection_algorithm.h"

#include <core/properties.h>

namespace cgp {

struct RouletteSelectionConfig : public core::PropertySet {
  PROPERTY(min_fitness,
           float,
           0.0f,
           "Minimum fitness for the genotypes participating the selection");

  PROPERTY(elite_percentage, float, 0.1f, "Elite percentage");
  PROPERTY(elite_min_fitness, float, 0.0f, "Elite minimum fitness");
};

class RouletteSelection : public selection::SelectionAlgorithm {
 public:
  explicit RouletteSelection(const core::PropertySet& config);

  void newPopulation(darwin::Population* population) override;
  void createNextGeneration(selection::GenerationFactory* next_generation) override;

 private:
  darwin::Population* population_ = nullptr;
  RouletteSelectionConfig config_;
};

}  // namespace cgp
