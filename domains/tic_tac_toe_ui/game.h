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

#include <domains/tic_tac_toe/board.h>
#include <domains/tic_tac_toe/player.h>

#include <vector>
using namespace std;

namespace tic_tac_toe_ui {

class Game {
 public:
  // game setup
  void newGame(tic_tac_toe::Player* x_player, tic_tac_toe::Player* o_player);
  void reset();

  // current player's turn, returns true if the game made progress
  bool takeTurn();

  const tic_tac_toe::Board& board() const { return board_; }

  tic_tac_toe::Player* currentPlayer() const;

  // navigating game history
  void historyToFirst();
  void historyToLast();
  void historyToPrevious();
  void historyToNext();

  // most current move, or Board::kNoSquare
  int lastMove() const;

 private:
  void applyMove(int move);
  void undoMove(int move);

 private:
  tic_tac_toe::Player* x_player_ = nullptr;
  tic_tac_toe::Player* o_player_ = nullptr;

  tic_tac_toe::Board board_;

  // game history
  vector<int> moves_history_;
  size_t active_moves_ = 0;
};

}  // namespace tic_tac_toe_ui
