// Copyright 2018 The Darwin Neuroevolution Framework Authors.
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

#include "ann_utils.h"
#include "utils.h"
#include "darwin.h"

#include <algorithm>
#include <cmath>
#include <random>
using namespace std;

namespace ann {

void initAnnLibrary();

//! Reset the values in a vector to 0
template <class T>
void reset(std::vector<T>& v) {
  std::fill(v.begin(), v.end(), T());
}

//! Reset the values in a Matrix
inline void reset(Matrix& matrix) {
  reset(matrix.values);
}

typedef void (*EvaluateLayer)(const vector<float>& in,
                              vector<float>& out,
                              const Matrix& w);

//! Evaluate a fully connected layer
extern EvaluateLayer evaluateLayer;

//! Apply the activation function over a set of values
//! \sa ann::activate()
inline void activateLayer(vector<float>& out) {
  for (float& value : out)
    value = ann::activate(value);
}

//! Randomize the values in a Matrix
//! 
//! - The random values are in the [-Config::connection_range, Config::connection_range]
//! - The values are rounded using ann::roundWeight()
//! - If Config::sparse_weights is true, only a subset of values are non-zero (subject to
//!   the Config::weights_density value)
//! 
inline void randomize(Matrix& w) {
  const float range = g_config.connection_range;

  std::random_device rd;
  std::default_random_engine rnd(rd());
  std::uniform_real_distribution<float> dist(-range, range);

  if (g_config.sparse_weights) {
    std::bernoulli_distribution density(g_config.weights_density);
    for (float& value : w.values)
      value = density(rnd) ? ann::roundWeight(dist(rnd)) : 0;
  } else {
    for (float& value : w.values)
      value = ann::roundWeight(dist(rnd));
  }
}

}  // namespace ann
