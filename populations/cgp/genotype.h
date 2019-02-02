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

// NOTE: removal/reordering of function IDs will break the
//  serialization format compatibility!
//
enum class FunctionId : uint16_t {
  // basic constants
  ConstZero,
  ConstOne,
  ConstTwo,
  
  // transcendental constants
  ConstPi,
  ConstE,
  
  // basic arithmetic functions
  Identity,
  Add,
  Subtract,
  Multiply,
  Divide,
  Negate,
  
  // extra arithmetic functions
  Fmod,
  Reminder,
  Fdim,
  Ceil,
  Floor,
  
  // common math functions
  Abs,
  Average,
  Min,
  Max,
  Square,
  
  // extra (mostly transcendental) math functions
  Log,
  Log2,
  Sqrt,
  Power,
  Exp,
  Exp2,
  
  // trigonometric functions
  Sin,
  Cos,
  Tan,
  Asin,
  Acos,
  Atan,
  
  // hyperbolic functions
  Sinh,
  Cosh,
  Tanh,
  
  // ANN activation functions
  AfnIdentity,
  AfnLogistic,
  AfnTanh,
  AfnReLU,
  AfnNeat,
  
  // comparisons
  CmpEq,
  CmpNe,
  CmpGt,
  CmpGe,
  CmpLt,
  CmpLe,
  
  // boolean logic gates
  And,
  Or,
  Not,
  Xor,
  
  // conditional
  IfOrZero,
  
  // last value marker
  LastEntry
};

constexpr int kFunctionCount = static_cast<int>(FunctionId::LastEntry);

struct FunctionGene {
  FunctionId function;
  array<IndexType, kMaxFunctionArity> connections;
  
  friend void to_json(json& json_obj, const FunctionGene& gene);
  friend void from_json(const json& json_obj, FunctionGene& gene);
  friend bool operator==(const FunctionGene& a, const FunctionGene& b);
};

struct OutputGene {
  IndexType connection;
  
  friend void to_json(json& json_obj, const OutputGene& gene);
  friend void from_json(const json& json_obj, OutputGene& gene);
  friend bool operator==(const OutputGene& a, const OutputGene& b);
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

  const Population* population() const { return population_; }
  const vector<FunctionGene>& functionGenes() const { return function_genes_; }
  const vector<OutputGene>& outputGenes() const { return output_genes_; }

  friend bool operator==(const Genotype& a, const Genotype& b);
  
 private:
  pair<IndexType, IndexType> connectionRange(int layer, int levels_back) const;

 private:
  const Population* population_ = nullptr;

  vector<FunctionGene> function_genes_;
  vector<OutputGene> output_genes_;
};

}  // namespace cgp
