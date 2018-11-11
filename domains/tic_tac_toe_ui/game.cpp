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

#include "game.h"
using namespace tic_tac_toe;

#include <assert.h>
#include <limits>
#include <utility>
using namespace std;

namespace tic_tac_toe_ui {

void Game::newGame(Player* x_player, Player* o_player) {
  CHECK(x_player != o_player);
  x_player_ = x_player;
  o_player_ = o_player;
  reset();
}

void Game::reset() {
  x_player_->newGame(&board_, Board::Piece::X);
  o_player_->newGame(&board_, Board::Piece::Zero);
  board_.reset();
  moves_history_.clear();
  active_moves_ = 0;
}

bool Game::takeTurn() {
  // is the game finished?
  if (board_.outcome() != Board::Outcome::Undecided)
    return false;

  // ask the current player to select a move
  int selected_move = currentPlayer()->move();
  if (selected_move == Board::kNoSquare)
    return false;
  applyMove(selected_move);
  return true;
}

Player* Game::currentPlayer() const {
  return active_moves_ % 2 ? o_player_ : x_player_;
}

void Game::historyToFirst() {
  while (active_moves_ > 0) {
    undoMove(moves_history_[active_moves_ - 1]);
  }
}

void Game::historyToLast() {
  while (active_moves_ < moves_history_.size()) {
    applyMove(moves_history_[active_moves_]);
  }
}

void Game::historyToPrevious() {
  if (active_moves_ > 0) {
    undoMove(moves_history_[active_moves_ - 1]);
  }
}

void Game::historyToNext() {
  if (active_moves_ < moves_history_.size()) {
    applyMove(moves_history_[active_moves_]);
  }
}

int Game::lastMove() const {
  CHECK(active_moves_ <= moves_history_.size());
  return active_moves_ > 0 ? moves_history_[active_moves_ - 1] : Board::kNoSquare;
}

void Game::applyMove(int move) {
  CHECK(active_moves_ <= moves_history_.size());
  CHECK(move >= 0 && move < Board::kSize);
  CHECK(board_[move] == Board::Piece::Empty);
  board_[move] = active_moves_ % 2 ? Board::Piece::Zero : Board::Piece::X;
  if (active_moves_ == moves_history_.size() || moves_history_[active_moves_] != move) {
    moves_history_.resize(active_moves_ + 1);
    moves_history_[active_moves_] = move;
  }
  ++active_moves_;
}

void Game::undoMove(int move) {
  CHECK(active_moves_ > 0);
  CHECK(move >= 0 && move < Board::kSize);
  CHECK(board_[move] != Board::Piece::Empty);
  board_[move] = Board::Piece::Empty;
  --active_moves_;
}

}  // namespace tic_tac_toe_ui
