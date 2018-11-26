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

#include <domains/tic_tac_toe/player.h>

namespace tic_tac_toe_ui {

class HumanPlayer : public tic_tac_toe::Player {
 public:
  int move() override {
    auto move = selected_move_;
    selected_move_ = tic_tac_toe::Board::kNoSquare;
    return move;
  }

  void selectMove(int move) {
    CHECK(move >= 0 && move < tic_tac_toe::Board::kSize);
    CHECK(selected_move_ == tic_tac_toe::Board::kNoSquare);

    const auto& board = *board_;
    if (board[move] == tic_tac_toe::Board::Piece::Empty &&
        board.state() == tic_tac_toe::Board::State::Undecided) {
      selected_move_ = move;
    }
  }

  string name() const override { return "Human"; }

 private:
  int selected_move_ = tic_tac_toe::Board::kNoSquare;
};

}  // namespace tic_tac_toe_ui
