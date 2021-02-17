// Copyright The Darwin Neuroevolution Framework Authors.
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

#include <core/utils.h>

#include <random>
using namespace std;

namespace core {

//! Returns an iterator to a random element in the input container
//!
//! \note The container must not be empty
//!
template <class T>
auto randomElem(T& container) {
  CHECK(!container.empty());
  random_device rd;
  default_random_engine rnd(rd());
  uniform_int_distribution<size_t> dist(0, container.size() - 1);
  return container.begin() + dist(rnd);
}

//! Returns an interator to a random element, where each element has an associated weight
//! (weight is considered to be the element's relative probability)
//!
//! The element type must have a `weight` member (`weight >= 0`)
//!
//! \note The container must not be empty
//!
template <class T>
auto randomWeightedElem(T& container) {
  CHECK(!container.empty());

  double total = 0;
  for (const auto& value : container) {
    CHECK(value.weight >= 0);
    total += value.weight;
  }
  CHECK(total > 0);

  random_device rd;
  default_random_engine rnd(rd());
  uniform_real_distribution<double> dist(0, total);
  const double sample = dist(rnd);

  // CONSIDER: the linear scan could be replaced with a binary search if needed
  double prefix_sum = 0;
  for (auto it = container.begin(); it != container.end(); ++it) {
    prefix_sum += it->weight;
    if (sample < prefix_sum) {
      return it;
    }
  }

  FATAL("Unreachable");
}

//! Mutates a floating point value using an normal distribution
template <class Scalar>
Scalar mutateNormalValue(Scalar value, Scalar std_dev) {
  random_device rd;
  default_random_engine rnd(rd());
  normal_distribution<Scalar> dist(value, std_dev);
  return dist(rnd);
}

//! Clamps a value between the specified upper and lower bounds
template <class Scalar>
Scalar clampValue(Scalar value, Scalar min_value, Scalar max_value) {
  CHECK(min_value <= max_value);
  return value > max_value ? max_value : (value < min_value ? min_value : value);
}

}  // namespace core
