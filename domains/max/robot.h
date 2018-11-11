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

#include <core/darwin.h>

#include <memory>
using namespace std;

namespace find_max_value {

struct World;

struct Robot {
  static constexpr int kInputs = 3;
  static constexpr int kOutputs = 3;

  enum class Action { None, MoveLeft, MoveRight, Done };

  static constexpr int kInputLeftAntena = 0;
  static constexpr int kInputRightAntena = 1;
  static constexpr int kInputValue = 2;

  static constexpr int kOutputMoveLeft = 0;
  static constexpr int kOutputMoveRight = 1;
  static constexpr int kOutputDone = 2;

  unique_ptr<darwin::Brain> brain;

  int pos = 0;
  int health = 0;
  float fitness = 0;

  const World* world = nullptr;

  void grow(const darwin::Genotype* genotype);
  bool alive() const { return health > 0; }

  void simInit(const World* new_world);
  void simStep();

 private:
  Action decideAction() const;
};

}  // namespace find_max_value
