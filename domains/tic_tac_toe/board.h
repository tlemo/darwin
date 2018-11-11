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

#include <core/properties.h>

#include <array>
using namespace std;

namespace tic_tac_toe {

// A 3x3 tic-tac-toe board, with the following layout:
//
//   0 | 1 | 2
// ----+---+----
//   3 | 4 | 5
// ----+---+----
//   6 | 7 | 8
//
// TODO: outcome checks can be made faster by using row/col/diag counters
//
class Board {
 public:
  static constexpr int kLines[][3] = {
    // rows
    { 0, 1, 2 },
    { 3, 4, 5 },
    { 6, 7, 8 },
    // columns
    { 0, 3, 6 },
    { 1, 4, 7 },
    { 2, 5, 8 },
    // diagonals
    { 0, 4, 8 },
    { 2, 4, 6 },
  };

  enum class Piece : char { Empty, X, Zero };
  enum class Outcome { Undecided, X_Wins, Zero_Wins, Draw };

  static constexpr int kSize = 9;
  static constexpr int kNoSquare = -1;

 public:
  Board() { reset(); }

  void reset() {
    for (auto& square : board_)
      square = Piece::Empty;
  }

  Piece& operator[](int index) { return board_[index]; }

  const Piece& operator[](int index) const { return board_[index]; }

  Outcome outcome() const;

  static Piece otherSide(Piece piece);

 private:
  array<Piece, kSize> board_;
};

}  // namespace tic_tac_toe
