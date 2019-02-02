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

#include "robot.h"

#include <core/properties.h>

#include <memory>
using namespace std;

namespace find_max_value {

constexpr int kMinSize = 5;

//! Find_Max_Value domain configuration
struct Config : public core::PropertySet {
  PROPERTY(min_size, int, 5, "Smallest array size");
  PROPERTY(max_size, int, 50, "Largest array size");

  PROPERTY(max_value, int, 100, "Range of values");

  PROPERTY(easy_map, bool, true, "Generate a sparse array (just a few non-zero values)");
  PROPERTY(test_worlds, int, 10, "Number of test worlds per generation");
};

extern Config g_config;

struct World {
 public:
  int map(int index) const { return map_[index]; }

  int goal() const { return goal_; }
  int size() const { return int(map_.size()); }

  void generate();
  bool fullyExplored() const;
  void simInit(const World& world, Robot* robot);
  void simStep();

 private:
  vector<int> map_;
  vector<char> visited_;
  int goal_ = 0;

  Robot* robot_;
};

}  // namespace find_max_value
