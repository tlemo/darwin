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

#include <chrono>

namespace core {

//! Automatic (scope based) wall-clock time measurements (in milliseconds)
class Chronometer {
 public:
  using Clock = std::chrono::steady_clock;

 public:
  explicit Chronometer(double* elapsed_ms, bool cumulative = false)
      : elapsed_ms_(elapsed_ms), cumulative_(cumulative) {
    start_timestamp_ = Clock::now();
  }

  ~Chronometer() {
    const auto finish_timestamp = Clock::now();
    const std::chrono::duration<double> d = finish_timestamp - start_timestamp_;
    *elapsed_ms_ = (cumulative_ ? *elapsed_ms_ : 0) + d.count() * 1000.0;
  }

 private:
  Clock::time_point start_timestamp_;
  double* elapsed_ms_ = nullptr;
  bool cumulative_ = false;
};

}  // namespace core
