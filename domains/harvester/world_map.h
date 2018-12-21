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

#include <core/math_2d.h>
#include <core/matrix.h>
#include <core/properties.h>

#include <algorithm>
#include <vector>
using namespace std;

namespace harvester {

//! Harvester domain configuration
struct Config : public core::PropertySet {
  PROPERTY(test_maps, int, 5, "Number of test maps");

  // map configuration
  PROPERTY(map_width, int, 64, "Map width");
  PROPERTY(map_height, int, 64, "Map height");

  PROPERTY(map_walls, int, 70, "Number of generated walls");
  PROPERTY(map_good_fruits, int, 40, "Number of 'good' fruits");
  PROPERTY(map_junk_fruits, int, 30, "Number of 'junk' fruits");
  PROPERTY(map_bad_fruits, int, 30, "Number of 'bad' fruits");

  // robot configuration
  PROPERTY(vision_resolution, int, 3, "Number of vision rays");
  PROPERTY(vision_fov,
           double,
           math::degreesToRadians(60.0),
           "Vision field of view (in radians)");

  PROPERTY(robot_size, double, 0.6, "Robot size");

  PROPERTY(exclusive_actuators,
           bool,
           true,
           "At each step, the robot can either move or turn, but not both");

  PROPERTY(move_speed, double, 1, "Move speed");
  PROPERTY(rotation_speed,
           double,
           math::degreesToRadians(3.0),
           "Rotation speed (in radians)");

  // health parameters
  PROPERTY(initial_health, int, 1000, "Initial health");
  PROPERTY(forward_move_drain, double, 2, "Health drain when moving forward");
  PROPERTY(reverse_move_drain, double, 6, "Health drain when moving backward");
  PROPERTY(good_fruit_health, int, 30, "Health update when eating a 'good' fruit");
  PROPERTY(junk_fruit_health, int, 5, "Health update when eating a 'junk' fruit");
  PROPERTY(bad_fruit_health, int, -100, "Health update when eating a 'bad' fruit");
};

extern Config g_config;

struct WorldMap {
  enum class Cell : char { Empty, Visited, FruitGood, FruitBad, FruitJunk, Wall };

  struct Pos {
    size_t row = -1;
    size_t col = -1;
  };

  core::Matrix<Cell> cells;

 public:
  WorldMap(int height, int width) : cells(height, width) {
    for (auto& cell : cells.values)
      cell = Cell::Empty;
  }

  bool generate(int max_attempts = 1000);

  Pos startPosition() const;

 private:
  bool isValid() const;
};

}  // namespace harvester
