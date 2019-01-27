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

#include <core/darwin.h>

#include <vector>
#include <array>
#include <cstdint>
#include <utility>
using namespace std;

namespace cgp {

class Population;

using IndexType = uint16_t;

constexpr int kMaxFunctionArity = 2;

enum class FunctionId : IndexType {
  // constants
  ConstZero,
  ConstOne,
  ConstTwo,
  ConstPi,
  ConstE,
  
  // arithmetic functions
  Add,
  Subtract,
  Multiply,
  Divide,
  
  // common math functions
  Abs,
  Average,
  Min,
  Max,
  Sin,
  Cos,
  Tan,
  Log,
  Sqrt,
  Square,
  Power,
  Exponential,
  
  // ANN activation functions
  Identity,     //!< Identity
  Logistic,     //!< Logistic
  Tanh,         //!< Hyperbolic tangent (tanh)
  ReLU,         //!< ReLU
  Neat,         //!< NEAT activation function
  
  // logic gates
  And,
  Or,
  Not,
  Xor,
  
  // last value marker
  LastEntry
};

constexpr int kFunctionCount = static_cast<int>(FunctionId::LastEntry);

struct FunctionGene {
  FunctionId function;
  array<IndexType, kMaxFunctionArity> connections;
};

struct OutputGene {
  IndexType connection;
};

class Genotype : public darwin::Genotype {
 public:
  explicit Genotype(const Population* population);

  unique_ptr<darwin::Brain> grow() const override;
  unique_ptr<darwin::Genotype> clone() const override;

  json save() const override;
  void load(const json& json_obj) override;
  void reset() override;

  void createPrimordialSeed();
  void mutate(float connection_mutation_chance, float function_mutation_chance);

  auto population() const { return population_; }
  auto functionGenes() const { return function_genes_; }
  auto outputGenes() const { return output_genes_; }
  
 private:
  pair<IndexType, IndexType> connectionRange(int layer) const;

 private:
  const Population* population_ = nullptr;

  vector<FunctionGene> function_genes_;
  vector<OutputGene> output_genes_;
};

}  // namespace cgp
