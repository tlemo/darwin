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

#include <third_party/box2d/box2d.h>

#include <vector>
using namespace std;

namespace sim {

//! A basic touch sensor attached to a physical body
//!
//! It maps collisions to sensors spaced evenly in slices around the body's coordinate
//! origin (sensor 0 starts at the positive y axis, advancing clockwise)
//!
class TouchSensor {
  static constexpr float kTouchSignal = 1.0f;
  static constexpr double kReceptorBoundaryTolerance = 0.1;

 public:
  TouchSensor(b2Body* body, int resolution);

  b2Body* body() const { return body_; }

  int resolution() const { return resolution_; }

  vector<float> receptors() const;

 private:
  double pointToSensor(const b2Vec2& point) const;

 private:
  b2Body* body_ = nullptr;
  const int resolution_ = 0;
};

}  // namespace sim
