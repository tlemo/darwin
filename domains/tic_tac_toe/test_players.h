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

#include "board.h"
#include "player.h"

#include <core/random.h>

#include <random>
using namespace std;

namespace tic_tac_toe {

class RandomPlayer : public Player {
 public:
  RandomPlayer(bool informed_choice) : informed_choice_(informed_choice) {}

  // Player interface
  int move() override;

  string name() const override;

 private:
  float evaluateMove(int square) const;

 private:
  mutable default_random_engine rnd_{ core::randomSeed() };
  bool informed_choice_ = false;
};

}  // namespace tic_tac_toe
