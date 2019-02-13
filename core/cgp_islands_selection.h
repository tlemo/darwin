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

#include <core/properties.h>
#include <core/selection_algorithm.h>

#include <vector>
using namespace std;

namespace selection {

struct CgpIslandsSelectionConfig : public core::PropertySet {
  PROPERTY(island_size, int, 10, "Size of the population islands");

  PROPERTY(protected_age,
           int,
           25,
           "The number of generations a new island is protected from extinction");

  PROPERTY(extinction_percentage,
           float,
           0.25f,
           "Percentage of low performing islands to go extinct");
};

class CgpIslandsSelection : public selection::SelectionAlgorithm {
  static constexpr int kPrimordialSeed = -1;
  
  struct Island {
    int age = 0;
    int parent = kPrimordialSeed;
  };

 public:
  explicit CgpIslandsSelection(const core::PropertySet& config);

  void newPopulation(darwin::Population* population) override;
  void createNextGeneration(selection::GenerationFactory* next_generation) override;

 private:
  darwin::Population* population_ = nullptr;
  CgpIslandsSelectionConfig config_;
  
  vector<Island> islands_;
};

}  // namespace selection
