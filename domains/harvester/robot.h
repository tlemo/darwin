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

#include "world_map.h"

#include <core/darwin.h>
#include <core/math_2d.h>

#include <memory>
#include <vector>
using namespace std;

namespace harvester {

class World;

// TODO: revisit the robot interface
class Robot {
  static constexpr int kOutputs = 2;

  static constexpr int kOutputMove = 0;
  static constexpr int kOutputRotate = 1;

  struct Ray {
    // relative to the robot's position
    math::Vector2d ray;

    // cached hit map cell
    int row = -1;
    int col = -1;

    Ray() = default;

    Ray(math::Scalar dx, math::Scalar dy, int row, int col)
        : ray(dx, dy), row(row), col(col) {}
  };

  struct Stats {
    double last_move_dist = 0;
    double total_move_dist = 0;
    int good_fruits = 0;
    int bad_fruits = 0;
    int junk_fruits = 0;
    int visited_cells = 0;
  };

 public:
  Robot();

  static int inputsCount() { return g_config.vision_resolution * 2; }
  static int outputsCount() { return kOutputs; }

  void grow(const darwin::Genotype* genotype, int initial_health);
  void simInit(World* world);
  void simStep();

  float fitness() const;
  bool alive() const { return health_ > 0; }
  int health() const { return health_; }

  const math::Vector2d& position() const { return pos_; }
  double angle() const { return angle_; }

  const Stats& stats() const { return stats_; }
  const vector<Ray>& vision() const { return vision_; }

 private:
  void resetState();
  void updateVision();
  Ray castRay(const math::Vector2d& v) const;

  void rotate(double angle);
  double move(double dist);
  void updateHealth(int value);

 private:
  World* world_ = nullptr;

  unique_ptr<darwin::Brain> brain_;

  int initial_health_ = 0;
  int health_ = 0;

  // robot position within the world
  math::Vector2d pos_;
  double angle_ = 0;

  // vision
  vector<Ray> vision_;

  Stats stats_;
};

}  // namespace harvester
