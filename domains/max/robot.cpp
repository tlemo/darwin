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

#include <assert.h>
#include <algorithm>
#include <cmath>
#include <iterator>
using namespace std;

namespace find_max_value {

void Robot::grow(const darwin::Genotype* genotype) {
  brain = genotype->grow();
  pos = 0;
  health = 0;
  fitness = 0;
}

void Robot::simInit(const World* new_world) {
  world = new_world;

  pos = world->size() / 2;
  health = world->size() * 4;
  fitness = 0;

  brain->resetState();
}

Robot::Action Robot::decideAction() const {
  float move_left = brain->output(kOutputMoveLeft);
  float move_right = brain->output(kOutputMoveRight);
  float done = brain->output(kOutputDone);

  auto action = Action::None;
  float max_activation = 0;

  if (move_left > max_activation) {
    action = Action::MoveLeft;
    max_activation = move_left;
  }

  if (move_right > max_activation) {
    action = Action::MoveRight;
    max_activation = move_right;
  }

  if (done > max_activation) {
    action = Action::Done;
    max_activation = done;
  }

  return action;
}

void Robot::simStep() {
  CHECK(alive());
  CHECK(pos >= 0 && pos < world->size());

  brain->setInput(kInputLeftAntena, pos == 0 ? 1.0f : 0.0f);
  brain->setInput(kInputRightAntena, pos == world->size() - 1 ? 1.0f : 0.0f);
  brain->setInput(kInputValue, float(world->map(pos)) / g_config.max_value);

  brain->think();
  --health;

  switch (decideAction()) {
    case Action::Done:
      CHECK(fitness == 0);
      if (world->map(pos) == world->goal())
        fitness += 60.0;
      if (world->fullyExplored())
        fitness += 40.0;
      health = 0;
      break;

    case Action::MoveLeft:
      if (pos > 0)
        --pos;
      break;

    case Action::MoveRight:
      if (pos + 1 < world->size())
        ++pos;
      break;

    case Action::None:
      // nothing to do
      break;
  }
}

}  // namespace find_max_value
