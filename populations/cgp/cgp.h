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

  PROPERTY(connection_mutation_chance,
           float,
           0.02f,
           "Probability of mutating a connection");
  PROPERTY(function_mutation_chance,
           float,
           0.01f,
           "Probability of mutating a node's function");

  PROPERTY(elite_percentage, float, 0.1f, "Elite percentage");
  PROPERTY(elite_min_fitness, float, 0.0f, "Elite minimum fitness");
};

}  // namespace cgp
