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
#include "world_map.h"

namespace harvester {

class World {
 public:
  World(const WorldMap& world_map, Robot* robot);

  World(const World&) = delete;
  World& operator=(const World&) = delete;

  void simInit();
  void simStep();

  const WorldMap& worldMap() const { return map_; }
  const Robot* robot() const { return robot_; }
  int currentStep() const { return step_; }

  // returns the previous cell type
  WorldMap::Cell visit(int row, int col);

 private:
  WorldMap map_;
  Robot* robot_ = nullptr;
  int step_ = -1;
};

}  // namespace harvester
