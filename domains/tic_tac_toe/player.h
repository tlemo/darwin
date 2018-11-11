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

#include <core/utils.h>

namespace tic_tac_toe {

class Player {
 public:
  Player() = default;
  virtual ~Player() = default;

  // start a new game
  virtual void newGame(const Board* board, Board::Piece side) {
    CHECK(side != Board::Piece::Empty);
    board_ = board;
    side_ = side;
  }

  // returns the index of the selected board square, or Board::kNoSquare if not ready
  virtual int move() = 0;

  virtual string name() const = 0;

  Board::Piece side() const { return side_; }

 protected:
  const Board* board_ = nullptr;
  Board::Piece side_ = Board::Piece::Empty;
};

}  // namespace tic_tac_toe
