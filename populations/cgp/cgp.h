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
#include "roulette_selection.h"
#include "cgp_islands_selection.h"
#include "truncation_selection.h"

#include <core/properties.h>

namespace cgp {

void init();

enum class SelectionAlgorithmType {
  RouletteWheel,
  CgpIslands,
  Truncation,
};

inline auto customStringify(core::TypeTag<SelectionAlgorithmType>) {
  static auto stringify = new core::StringifyKnownValues<SelectionAlgorithmType>{
    { SelectionAlgorithmType::RouletteWheel, "roulette_wheel" },
    { SelectionAlgorithmType::CgpIslands, "cgp_islands" },
    { SelectionAlgorithmType::Truncation, "truncation" },
  };
  return stringify;
}

struct SelectionAlgorithmVariant
    : public core::PropertySetVariant<SelectionAlgorithmType> {
  CASE(SelectionAlgorithmType::RouletteWheel, roulette_wheel, RouletteSelectionConfig);
  CASE(SelectionAlgorithmType::CgpIslands, cgp_islands, CgpIslandsSelectionConfig);
  CASE(SelectionAlgorithmType::Truncation, truncation, TrunctationSelectionConfig);
};

struct Config : public core::PropertySet {
  PROPERTY(rows, int, 2, "Number of node rows");
  PROPERTY(columns, int, 8, "Number of node columns");
  PROPERTY(levels_back, int, 4, "Levels-back");

  PROPERTY(outputs_use_levels_back,
           bool,
           false,
           "Use levels-back value for the output genes?");

  PROPERTY(fn_basic_constants, bool, true, "0, 1, 2");
  PROPERTY(fn_transcendental_constants, bool, true, "pi, e");
  PROPERTY(fn_basic_arithmetic, bool, true, "identity, +, -, *, /, negate");
  PROPERTY(fn_extra_arithmetic, bool, true, "fmod, reminder, ceil, floor, fdim");
  PROPERTY(fn_common_math, bool, true, "abs, avg, min, max, square");
  PROPERTY(fn_extra_math, bool, true, "log, log2, sqrt, power, exp, exp2");
  PROPERTY(fn_trigonometric, bool, true, "sin, cos, tan, asin, acos, atan");
  PROPERTY(fn_hyperbolic, bool, true, "sinh, cosh, tanh");
  PROPERTY(fn_ann_activation, bool, true, "All of Darwin's activation functions");
  PROPERTY(fn_comparisons, bool, true, "eq, ne, gt, ge, lt, le");
  PROPERTY(fn_logic_gates, bool, true, "and, or, not, xor");
  PROPERTY(fn_conditional, bool, true, "if/else_zero");
  PROPERTY(fn_stateful, bool, true, "functions maintaining an internal state");

  PROPERTY(evolvable_constants_count, int, 2, "The number of evolvable constants");
  PROPERTY(evolvable_constants_range, float, 10, "Initial connection values range");
  PROPERTY(evolvable_constants_resolution, float, 0.01f, "Connection values resolution");
  PROPERTY(evolvable_constants_std_dev,
           float,
           2.0f,
           "Mutation standard deviation, used for evolvable constants");

  VARIANT(mutation_strategy,
          MutationVariant,
          MutationStrategy::FixedCount,
          "Mutation strategy");

  VARIANT(selection_algorithm,
          cgp::SelectionAlgorithmVariant,
          cgp::SelectionAlgorithmType::RouletteWheel,
          "Selecton algorithm");
};

}  // namespace cgp
