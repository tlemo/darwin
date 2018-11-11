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
#include "world_map.h"

#include <assert.h>
using namespace std;

namespace harvester {

World::World(const WorldMap& world_map, Robot* robot) : map_(world_map), robot_(robot) {
  assert(robot_ != nullptr);
}

void World::simInit() {
  // make sure the map hasn't been already used
  //
  // TODO: redesign the World interface to avoid the need for this
  //
  for (const auto& cell : map_.cells.values)
    assert(cell != WorldMap::Cell::Visited);

  step_ = 0;
  robot_->simInit(this);
}

void World::simStep() {
  robot_->simStep();
  ++step_;
}

WorldMap::Cell World::visit(int row, int col) {
  auto orig_cell = map_.cells[row][col];
  if (orig_cell != WorldMap::Cell::Wall)
    map_.cells[row][col] = WorldMap::Cell::Visited;
  return orig_cell;
}

}  // namespace harvester
