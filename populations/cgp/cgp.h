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

namespace cgp {

void init();

struct Config : public core::PropertySet {
  PROPERTY(rows, int, 8, "Number of node rows");
  PROPERTY(columns, int, 16, "Number of node columns");
  PROPERTY(levels_back, int, 2, "Levels-back");

  PROPERTY(outputs_use_levels_back,
           bool,
           false,
           "Use levels-back value for the output genes?");

  PROPERTY(connection_mutation_chance,
           float,
           0.02f,
           "Probability of mutating a connection");
  PROPERTY(function_mutation_chance,
           float,
           0.01f,
           "Probability of mutating a node's function");

  PROPERTY(fn_basic_constants, bool, true, "0, 1, 2");
  PROPERTY(fn_transcendental_constants, bool, true, "pi, e");
  PROPERTY(fn_basic_arithmetic, bool, true, "+, -, *, /, negate");
  PROPERTY(fn_extra_arithmetic, bool, false, "fmod, reminder, ceil, floor, fdim");
  PROPERTY(fn_common_math, bool, true, "abs, avg, min, max, square");
  PROPERTY(fn_extra_math, bool, false, "log, log2, sqrt, power, exp, exp2");
  PROPERTY(fn_trigonometric, bool, false, "sin, cos, tan, asin, acos, atan");
  PROPERTY(fn_hyperbolic, bool, false, "sinh, cosh, tanh");
  PROPERTY(fn_ann_activation, bool, false, "All of Darwin's activation functions");
  PROPERTY(fn_comparisons, bool, false, "eq, ne, gt, ge, lt, le");
  PROPERTY(fn_logic_gates, bool, false, "and, or, not, xor");
  PROPERTY(fn_conditional, bool, false, "if/else_zero");

  PROPERTY(elite_percentage, float, 0.1f, "Elite percentage");
  PROPERTY(elite_min_fitness, float, 0.0f, "Elite minimum fitness");
};

}  // namespace cgp
