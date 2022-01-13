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

#include "world.h"
#include "robot.h"

#include <core/random.h>

#include <assert.h>
#include <algorithm>
#include <deque>
#include <iterator>
#include <limits>
#include <memory>
#include <random>
using namespace std;

namespace find_max_value {

Config g_config;

void World::generate() {
  CHECK(g_config.min_size >= kMinSize);

  default_random_engine rnd(core::randomSeed());

  uniform_int_distribution<int> dist_size(g_config.min_size, g_config.max_size);
  uniform_int_distribution<int> dist_val(1, g_config.max_value);

  map_.resize(dist_size(rnd));

  if (g_config.easy_map) {
    for (auto& value : map_)
      value = 0;

    map_[0] = dist_val(rnd);
    map_[1] = dist_val(rnd);
    map_[2] = dist_val(rnd);
    map_[3] = dist_val(rnd);
  } else {
    int base = dist_val(rnd);
    for (size_t i = 0; i < map_.size(); ++i)
      map_[i] = base + int(i);
  }

  shuffle(map_.begin(), map_.end(), rnd);
  goal_ = *max_element(map_.begin(), map_.end());
}

bool World::fullyExplored() const {
  CHECK(visited_.size() == map_.size());
  for (bool visited_cell : visited_) {
    if (!visited_cell)
      return false;
  }
  return true;
}

void World::simInit(const World& world, Robot* robot) {
  map_ = world.map_;
  visited_.resize(map_.size());
  std::fill(visited_.begin(), visited_.end(), false);
  goal_ = world.goal_;

  robot_ = robot;
  robot_->simInit(this);
}

void World::simStep() {
  assert(robot_->world == this);

  CHECK(robot_->pos >= 0 && robot_->pos < map_.size());
  visited_[robot_->pos] = true;
  robot_->simStep();
}

}  // namespace find_max_value
