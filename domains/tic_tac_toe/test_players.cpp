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

#include "test_players.h"

#include <core/utils.h>

#include <limits>
using namespace std;

namespace tic_tac_toe {

int RandomPlayer::move() {
  CHECK(side_ != Board::Piece::Empty);

  int best_move = -1;
  float best_move_value = -numeric_limits<float>::infinity();

  const auto& board = *board_;
  for (int square = 0; square < Board::kSize; ++square) {
    if (board[square] == Board::Piece::Empty) {
      float move_value = evaluateMove(square);
      if (move_value > best_move_value) {
        best_move_value = move_value;
        best_move = square;
      }
    }
  }

  CHECK(best_move != -1);
  return best_move;
}

float RandomPlayer::evaluateMove(int square) const {
  const auto& board = *board_;
  CHECK(board[square] == Board::Piece::Empty);

  struct MoveValueRange {
    float min_value;
    float max_value;
  };

  // the idea is to have the values "fair" within the same range,
  // but more desirable move types are placed in strictly ordered
  // ranges (ex a winning move will always have a higer value than
  // either a completely random or a blocking move)
  constexpr MoveValueRange kRandomMoveRange = { 0, 1 };
  constexpr MoveValueRange kBlockingMoveRange = { 2, 3 };
  constexpr MoveValueRange kWinningMoveRange = { 4, 5 };

  MoveValueRange value_range = kRandomMoveRange;

  if (informed_choice_) {
    bool winning = false;
    bool blocking = false;

    // check every line containing the move square
    for (const auto& line : Board::kLines) {
      int mine = 0;
      int opponent = 0;
      bool relevant = false;

      for (int line_square : line) {
        if (line_square == square) {
          relevant = true;
          ++mine;
        } else if (board[line_square] == side_) {
          ++mine;
        } else if (board[line_square] == Board::otherSide(side_)) {
          ++opponent;
        }
      }

      if (relevant) {
        CHECK(mine > 0);
        CHECK(mine + opponent <= 3);

        if (mine == 3)
          winning = true;
        else if (mine == 1 && opponent == 2)
          blocking = true;
      }
    }

    if (winning)
      value_range = kWinningMoveRange;
    else if (blocking)
      value_range = kBlockingMoveRange;
  }

  uniform_real_distribution<float> dist(value_range.min_value, value_range.max_value);
  return dist(rnd_);
}

string RandomPlayer::name() const {
  return informed_choice_ ? "Informed Random" : "Random";
}

}  // namespace tic_tac_toe
