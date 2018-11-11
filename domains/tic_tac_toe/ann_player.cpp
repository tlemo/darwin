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

#include "ann_player.h"
#include "tic_tac_toe.h"

#include <core/utils.h>
#include <core/format.h>

#include <limits>
using namespace std;

namespace tic_tac_toe {

static float pieceValue(Board::Piece piece, Board::Piece side) {
  switch (piece) {
    case Board::Piece::Empty:
      return 0;

    case Board::Piece::X:
    case Board::Piece::Zero:
      // own squares are 1, opponent -1
      return piece == side ? 1 : -1;

    default:
      FATAL("unexpected piece");
  }
}

int AnnPlayer::outputs() {
  switch (g_config.ann_type) {
    case AnnType::Policy:
      return 9;
    case AnnType::Value:
      return 1;
    default:
      FATAL("Unexpected ANN type");
  }
}

void AnnPlayer::grow(const darwin::Genotype* genotype, int generation) {
  generation_ = generation;
  brain_ = genotype->grow();
  genotype_ = genotype;
}

void AnnPlayer::newGame(const Board* board, Board::Piece side) {
  Player::newGame(board, side);
  brain_->resetState();
}

int AnnPlayer::move() {
  CHECK(side_ != Board::Piece::Empty);

  switch (g_config.ann_type) {
    case AnnType::Policy:
      return policyBrainMove();

    case AnnType::Value:
      return valueBrainMove();

    default:
      FATAL("Unexpected ANN type");
  }
}

int AnnPlayer::policyBrainMove() {
  const auto& board = *board_;

  for (int i = 0; i < Board::kSize; ++i) {
    brain_->setInput(i, pieceValue(board[i], side_));
  }

  brain_->think();

  // pick the legal move with the largest output signal
  //
  // TODO: track invalid preferences: max signal is not a valid move
  //
  float best_move_signal = -numeric_limits<float>::infinity();
  int best_move = -1;
  for (int i = 0; i < Board::kSize; ++i) {
    const float signal = brain_->output(i);
    if (signal > best_move_signal && board[i] == Board::Piece::Empty) {
      best_move_signal = signal;
      best_move = i;
    }
  }

  CHECK(best_move != -1);
  return best_move;
}

int AnnPlayer::valueBrainMove() {
  int best_move = -1;
  float best_move_value = -numeric_limits<float>::infinity();

  const auto& board = *board_;
  for (int square = 0; square < Board::kSize; ++square) {
    if (board[square] == Board::Piece::Empty) {
      brain_->resetState();
      
      for (int i = 0; i < Board::kSize; ++i) {
        brain_->setInput(i, i == square ? 1 : pieceValue(board[i], side_));
      }

      brain_->think();

      const float move_value = brain_->output(0);
      if (move_value > best_move_value) {
        best_move_value = move_value;
        best_move = square;
      }
    }
  }

  CHECK(best_move != -1);
  return best_move;
}

string AnnPlayer::name() const {
  return core::format("ANN Gen %d", generation_);
}

}  // namespace tic_tac_toe
