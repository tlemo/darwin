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

#include "player.h"

#include <core/darwin.h>

namespace pong {

class AnnPlayer : public Player {
 public:
  static constexpr int kInputs = 6;
  static constexpr int kOutputs = 2;

  struct Stats {
    int won = 0;
    int lost = 0;
    int drawn = 0;
  };

 private:
  static constexpr int kInputMyPaddlePos = 0;
  static constexpr int kInputOpponentPaddlePos = 1;
  static constexpr int kInputBallX = 2;
  static constexpr int kInputBallY = 3;
  static constexpr int kInputBallVx = 4;
  static constexpr int kInputBallVy = 5;

  static constexpr int kOutputMoveUp = 0;
  static constexpr int kOutputMoveDown = 1;

 public:
  unique_ptr<darwin::Brain> brain;
  const darwin::Genotype* genotype = nullptr;
  Stats stats;
  int generation = -1;

 public:
  Action action() override;
  string name() const override;
  void newGame(const Game* game, Side side) override;

  void grow(const darwin::Genotype* genotype);
};

}  // namespace pong
