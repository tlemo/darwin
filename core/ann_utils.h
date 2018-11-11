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

#include "ann_activation_functions.h"
#include "utils.h"
#include "matrix.h"
#include "properties.h"

#include <assert.h>
#include <cmath>
#include <random>
#include <vector>
using namespace std;

namespace ann {

extern bool g_use_AVX;

//! The ANN library configuration
struct Config : public core::PropertySet {
  PROPERTY(mutation_normal_distribution,
           bool,
           true,
           "Use normal (instead of uniform) distribution for mutations");

  PROPERTY(mutation_std_dev,
           float,
           1.0f,
           "Mutation standard deviation. Used if mutation_normal_distribution is true.");

  PROPERTY(connection_range, float, 64, "Initial connection values range");
  PROPERTY(connection_resolution, float, 0.01f, "Connection values resolution");

  PROPERTY(sparse_weights,
           bool,
           false,
           "Generate sparse weights for the initial (random) population");

  PROPERTY(weights_density,
           float,
           0.2f,
           "Probability of non-zero weights (if sparse_weights is true)");
};

// global configuration values
// (shall not be changed while the evolution is running)
extern Config g_config;

//! Ajust a value by rounding to Config::connection_resolution
inline float roundWeight(float w) {
  const float resolution = g_config.connection_resolution;
  return int(w / resolution) * resolution;
}

//! Mutate a value
//! \sa Config::mutation_normal_distribution
//! \sa Config::connection_range
//! \sa roundWeight()
template <class T, class RND>
void mutateValue(T& value, RND& rnd, T std_dev) {
  if (g_config.mutation_normal_distribution) {
    std::normal_distribution<T> dist(value, std_dev);
    value = roundWeight(dist(rnd));
  } else {
    const float range = g_config.connection_range;
    std::uniform_real_distribution<T> dist(-range, range);
    value = roundWeight(dist(rnd));
  }
}

// CONSIDER: do we really need this alias?
using Matrix = core::Matrix<float>;

}  // namespace ann
