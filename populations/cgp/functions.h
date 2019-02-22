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

#include <core/stringify.h>

#include <cstdint>
using namespace std;

namespace cgp {

constexpr int kMaxFunctionArity = 2;

enum FunctionId : int16_t {
  #undef FN_DEF
  #define FN_DEF(id, name, arity, category) id,
  #include "functions_table.def"
  LastEntry
};

constexpr int kFunctionCount = static_cast<int>(FunctionId::LastEntry);

inline auto customStringify(core::TypeTag<FunctionId>) {
  static auto stringify = new core::StringifyKnownValues<FunctionId>{
    #undef FN_DEF
    #define FN_DEF(id, name, arity, category) { FunctionId::id, #name },
    #include "functions_table.def"
  };
  return stringify;
}

enum class FunctionCategory {
  BasicConstant,
  TranscendentalConstant,
  BasicArithmetic,
  ExtraArithmetic,
  CommonMath,
  ExtraMath,
  Trigonometric,
  Hyperbolic,
  AnnActivation,
  Comparisons,
  LogicGates,
  Conditional,
  Stateful,
};

struct FunctionDef {
  const FunctionId id;
  const char* const name;
  const int arity;
  const FunctionCategory category;
};

constexpr FunctionDef kFunctionDef[kFunctionCount] = {
  #undef FN_DEF
  #define FN_DEF(id, name, arity, category) \
    { id, #name, (arity), FunctionCategory::category },
  #include "functions_table.def"
};

}  // namespace cgp
