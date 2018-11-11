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

#include "robot.h"
#include "world.h"
#include "world_map.h"

#include <assert.h>
#include <algorithm>
#include <cmath>
using namespace std;

namespace harvester {

Robot::Robot() {
  CHECK(g_config.vision_resolution > 0);
  vision_.resize(g_config.vision_resolution);
}

float Robot::fitness() const {
  return stats_.good_fruits;
}

void Robot::grow(const darwin::Genotype* genotype, int initial_health) {
  CHECK(initial_health > 0);
  initial_health_ = initial_health;
  brain_ = genotype->grow();
}

void Robot::resetState() {
  assert(world_ != nullptr);
  assert(initial_health_ > 0);

  auto start_pos = world_->worldMap().startPosition();
  pos_ = math::Vector2d(start_pos.col + 0.5, start_pos.row + 0.5);
  angle_ = 0;
  health_ = initial_health_;
  stats_ = {};
}

void Robot::simInit(World* world) {
  assert(world != nullptr);
  world_ = world;

  resetState();
  brain_->resetState();

  int row = int(pos_.y);
  int col = int(pos_.x);
  world_->visit(row, col);

  updateVision();
}

void Robot::rotate(double angle) {
  // clamp angle
  angle = fmin(angle, math::kPi);
  angle = fmax(angle, -math::kPi);

  angle_ += angle;
}

// returns the actual traveled distance
double Robot::move(double dist) {
  // clamp dist
  dist = fmin(dist, 0.9);
  dist = fmax(dist, -0.9);

  math::HMatrix2d hm;
  hm.setRotation(angle_);

  math::Vector2d new_pos = pos_ + hm * math::Vector2d(dist, 0);

  int col = int(new_pos.x);
  int row = int(new_pos.y);

  switch (world_->visit(row, col)) {
    case WorldMap::Cell::Wall:
      return 0;

    case WorldMap::Cell::FruitBad:
      updateHealth(g_config.bad_fruit_health);
      ++stats_.bad_fruits;
      ++stats_.visited_cells;
      break;

    case WorldMap::Cell::FruitJunk:
      updateHealth(g_config.junk_fruit_health);
      ++stats_.junk_fruits;
      ++stats_.visited_cells;
      break;

    case WorldMap::Cell::FruitGood:
      updateHealth(g_config.good_fruit_health);
      ++stats_.good_fruits;
      ++stats_.visited_cells;
      break;

    case WorldMap::Cell::Empty:
      ++stats_.visited_cells;
      break;

    case WorldMap::Cell::Visited:
      // nothing to do
      break;
  }

  pos_ = new_pos;
  return dist;
}

void Robot::updateHealth(int value) {
  CHECK(health_ > 0 || value < 0);
  health_ += value;

  if (health_ <= 0) {
    // dead...
    health_ = 0;
  }
}

void Robot::simStep() {
  CHECK(alive());

  assert(world_ != nullptr);
  assert(brain_);

  brain_->think();

  auto rotation_angle = brain_->output(kOutputRotate) * g_config.rotation_speed;
  auto move_dist = brain_->output(kOutputMove) * g_config.move_speed;

  // actuators
  if (g_config.exclusive_actuators) {
    if (fabs(rotation_angle) >= fabs(move_dist)) {
      rotate(rotation_angle);
      stats_.last_move_dist = 0;
    } else {
      stats_.last_move_dist = move(move_dist);
    }
  } else {
    rotate(rotation_angle);
    stats_.last_move_dist = move(move_dist);
  }

  updateVision();

  stats_.total_move_dist += fabs(stats_.last_move_dist);

  // update health
  double move_drain = stats_.last_move_dist >= 0 ? g_config.forward_move_drain
                                                 : -g_config.reverse_move_drain;
  int health_drain = 1 + int(stats_.last_move_dist * move_drain);
  CHECK(health_drain > 0);
  updateHealth(-health_drain);
}

void Robot::updateVision() {
  math::HMatrix2d hm;
  math::Vector2d ray_vector(1, 0);

  if (g_config.vision_resolution > 1) {
    hm.setRotation(angle_ - g_config.vision_fov / 2);
    ray_vector = hm * ray_vector;

    hm.setRotation(g_config.vision_fov / (g_config.vision_resolution - 1));
  } else {
    hm.setRotation(angle_);
    ray_vector = hm * ray_vector;
  }

  const WorldMap& world_map = world_->worldMap();
  const math::Vector2d world_diagonal(world_map.cells.rows, world_map.cells.cols);
  const math::Scalar world_diagonal_length = world_diagonal.length();

  for (int i = 0; i < g_config.vision_resolution; ++i) {
    auto vision_ray = castRay(ray_vector);
    vision_[i] = vision_ray;

    float color = 0;
    switch (world_map.cells[vision_ray.row][vision_ray.col]) {
      case WorldMap::Cell::FruitBad:
        color = -1;
        break;
      case WorldMap::Cell::FruitJunk:
        color = 0.5;
        break;
      case WorldMap::Cell::FruitGood:
        color = 1;
        break;
      case WorldMap::Cell::Wall:
        color = 0;
        break;
      default:
        FATAL("unexpected cell type");
    }

    // two values per ray: { normalized distance, color }
    int input_index = i * 2;
    float dist = vision_ray.ray.length() / world_diagonal_length;
    brain_->setInput(input_index + 0, dist);
    brain_->setInput(input_index + 1, color);

    ray_vector = hm * ray_vector;
  }
}

static bool isEmpty(WorldMap::Cell cell) {
  return cell == WorldMap::Cell::Empty || cell == WorldMap::Cell::Visited;
}

Robot::Ray Robot::castRay(const math::Vector2d& v) const {
  math::Scalar dx = 0;
  math::Scalar dy = 0;
  int col = int(pos_.x);
  int row = int(pos_.y);

  const WorldMap& world_map = world_->worldMap();

  if (fabs(v.x) >= fabs(v.y)) {
    const math::Scalar slope = v.y / v.x;

    if (v.x > 0) {
      dx = (col + 1) - pos_.x;
      dy = dx * slope;

      for (++col; col < world_map.cells.cols; ++col, dy += slope, dx += 1) {
        if (row != int(dy + pos_.y)) {
          row = int(dy + pos_.y);
          if (!isEmpty(world_map.cells[row][col - 1])) {
            dy = (v.y > 0 ? row : row + 1) - pos_.y;
            dx = dy * (v.x / v.y);
            --col;
            break;
          }
        }

        if (!isEmpty(world_map.cells[row][col]))
          break;
      }
    } else {
      dx = col - pos_.x;
      dy = dx * slope;

      for (--col; col >= 0; --col, dy -= slope, dx -= 1) {
        if (row != int(dy + pos_.y)) {
          row = int(dy + pos_.y);
          if (!isEmpty(world_map.cells[row][col + 1])) {
            dy = (v.y > 0 ? row : row + 1) - pos_.y;
            dx = dy * (v.x / v.y);
            ++col;
            break;
          }
        }

        if (!isEmpty(world_map.cells[row][col]))
          break;
      }
    }
  } else {
    const math::Scalar slope = v.x / v.y;

    if (v.y > 0) {
      dy = (row + 1) - pos_.y;
      dx = dy * slope;

      for (++row; row < world_map.cells.rows; ++row, dx += slope, dy += 1) {
        if (col != int(dx + pos_.x)) {
          col = int(dx + pos_.x);
          if (!isEmpty(world_map.cells[row - 1][col])) {
            dx = (v.x > 0 ? col : col + 1) - pos_.x;
            dy = dx * (v.y / v.x);
            --row;
            break;
          }
        }

        if (!isEmpty(world_map.cells[row][col]))
          break;
      }
    } else {
      dy = row - pos_.y;
      dx = dy * slope;

      for (--row; row >= 0; --row, dx -= slope, dy -= 1) {
        if (col != int(dx + pos_.x)) {
          col = int(dx + pos_.x);
          if (!isEmpty(world_map.cells[row + 1][col])) {
            dx = (v.x > 0 ? col : col + 1) - pos_.x;
            dy = dx * (v.y / v.x);
            ++row;
            break;
          }
        }

        if (!isEmpty(world_map.cells[row][col]))
          break;
      }
    }
  }

  assert(!isEmpty(world_map.cells[row][col]));
  return Ray(dx, dy, row, col);
}

}  // namespace harvester
