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

#include "world_map.h"

#include <core/utils.h>
#include <core/logging.h>
#include <core/random.h>

#include <assert.h>
#include <algorithm>
#include <deque>
#include <random>
#include <vector>
using namespace std;

namespace harvester {

Config g_config;

// generate a random map
bool WorldMap::generate(int max_attempts) {
  CHECK(!cells.empty());

  default_random_engine rnd(core::randomSeed());
  uniform_int_distribution<size_t> dist_row(0, cells.rows - 1);
  uniform_int_distribution<size_t> dist_col(0, cells.cols - 1);
  uniform_int_distribution<size_t> dist_size(1, 10);

  // generate the walls
  bool valid = false;
  for (int attempts = 0; attempts < max_attempts; ++attempts) {
    for (size_t row = 0; row < cells.rows; ++row)
      for (size_t col = 0; col < cells.cols; ++col) {
        bool v_edge = (col == 0 || col == cells.cols - 1);
        bool h_edge = (row == 0 || row == cells.rows - 1);
        cells[row][col] = (v_edge || h_edge) ? Cell::Wall : Cell::Empty;
      }

    for (int wall = 0; wall < g_config.map_walls; ++wall) {
      size_t row = dist_row(rnd);
      size_t col = dist_col(rnd);
      size_t width = dist_size(rnd);
      size_t height = dist_size(rnd);

      width = std::min(width, cells.cols - col);
      height = std::min(height, cells.rows - row);

      for (size_t i = 0; i < height; ++i)
        for (size_t j = 0; j < width; ++j)
          cells[row + i][col + j] = Cell::Wall;
    }

    if (isValid()) {
      if (attempts > 50)
        core::log("WARNING: excessive world map generation attempts (%d)\n", attempts);

      valid = true;
      break;
    }
  }

  if (!valid)
    return false;

  // don't place any fruits on the starting cell
  const Pos start_pos = startPosition();

  auto placeFruits = [&](int count, Cell fruit) {
    while (count > 0) {
      size_t row = dist_row(rnd);
      size_t col = dist_col(rnd);

      if (col == start_pos.col || row == start_pos.row)
        continue;

      if (cells[row][col] != Cell::Empty)
        continue;

      cells[row][col] = fruit;
      --count;
    }
  };

  // generate the fruits
  placeFruits(g_config.map_good_fruits, Cell::FruitGood);
  placeFruits(g_config.map_junk_fruits, Cell::FruitJunk);
  placeFruits(g_config.map_bad_fruits, Cell::FruitBad);

  return true;
}

WorldMap::Pos WorldMap::startPosition() const {
  assert(!cells.empty());
  return Pos{ cells.rows / 2, cells.cols / 2 };
}

bool WorldMap::isValid() const {
  auto tmp_map = cells;
  assert(!tmp_map.empty());

  // flood fill the map starting from the start position
  deque<Pos> q;

  auto fill = [&](const Pos& pos) {
    CHECK(pos.row >= 0 && pos.row < tmp_map.rows);
    CHECK(pos.col >= 0 && pos.col < tmp_map.cols);
    if (tmp_map[pos.row][pos.col] == Cell::Empty) {
      tmp_map[pos.row][pos.col] = Cell::Visited;
      q.push_back(pos);
    }
  };

  fill(startPosition());
  while (!q.empty()) {
    auto pos = q.front();
    q.pop_front();
    fill(Pos{ pos.row, pos.col + 1 });
    fill(Pos{ pos.row, pos.col - 1 });
    fill(Pos{ pos.row + 1, pos.col });
    fill(Pos{ pos.row - 1, pos.col });
  }

  // check to see if we had any unreachable areas
  int empty_space = 0;
  for (const auto& cell : tmp_map.values)
    switch (cell) {
      case Cell::Wall:
        break;

      case Cell::Visited:
        ++empty_space;
        break;

      case Cell::Empty:
        return false;

      default:
        FATAL("unexpected map cell type");
    }

  return empty_space >= g_config.map_good_fruits + g_config.map_junk_fruits +
                            g_config.map_bad_fruits + 1;  // start cell
}

}  // namespace harvester
