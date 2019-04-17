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

#include <core/darwin.h>
#include <core/utils.h>

namespace selection {

class GenotypeFactory {
 public:
  virtual ~GenotypeFactory() = default;

  virtual void createPrimordialSeed() = 0;
  virtual void replicate(int parent_index) = 0;
  virtual void crossover(int parent1, int parent2, float preference) = 0;
  virtual void mutate() = 0;
};

class GenerationFactory {
 public:
  virtual ~GenerationFactory() = default;

  virtual size_t size() const = 0;
  virtual GenotypeFactory* operator[](size_t index) = 0;
};

//! Selection Algorithm interface
class SelectionAlgorithm : public core::NonCopyable {
 public:
  virtual ~SelectionAlgorithm() = default;
  
  //! Prepare the selection for a new population
  virtual void newPopulation(darwin::Population* population) = 0;
  
  //! Create a new generation of genotypes
  virtual void createNextGeneration(GenerationFactory* next_generation) = 0;
};

}  // namespace selection
