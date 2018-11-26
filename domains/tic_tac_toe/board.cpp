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

#include "board.h"

namespace tic_tac_toe {

// current position outcome (relative to X)
Board::State Board::state() const {
  for (const auto& line : kLines) {
    auto c0 = board_[line[0]];
    auto c1 = board_[line[1]];
    auto c2 = board_[line[2]];
    if (c0 != Piece::Empty && c0 == c1 && c1 == c2)
      return c0 == Piece::X ? State::X_Wins : State::Zero_Wins;
  }

  for (auto piece : board_)
    if (piece == Piece::Empty)
      return State::Undecided;

  return State::Draw;
}

Board::Piece Board::otherSide(Board::Piece piece) {
  switch (piece) {
    case Piece::X:
      return Piece::Zero;
    case Piece::Zero:
      return Piece::X;
    default:
      FATAL("unexpected piece type");
  }
}

}  // namespace tic_tac_toe
