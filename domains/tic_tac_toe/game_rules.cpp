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

#include "game_rules.h"
#include "ann_player.h"

namespace tic_tac_toe {

tournament::Scores TicTacToeRules::scores(tournament::GameOutcome outcome) const {
  switch (outcome) {
    case tournament::GameOutcome::FirstPlayerWins:
      return { +1, 0 };
    case tournament::GameOutcome::Draw:
      return { +0.5f, +0.5f };
    case tournament::GameOutcome::SecondPlayerWins:
      return { 0, +1 };
    default:
      FATAL("unexpected outcome");
  }
}

tournament::GameOutcome TicTacToeRules::play(Player* x_player, Player* o_player) const {
  Board board;

  x_player->newGame(&board, Board::Piece::X);
  o_player->newGame(&board, Board::Piece::Zero);

  // X plays first
  Player* current_player = x_player;
  Player* next_player = o_player;

  for (;;) {
    int selected_move = current_player->move();
    CHECK(selected_move != Board::kNoSquare);
    CHECK(board[selected_move] == Board::Piece::Empty);
    board[selected_move] = current_player->side();

    switch (board.state()) {
      case Board::State::Undecided:
        break;
      case Board::State::X_Wins:
        return tournament::GameOutcome::FirstPlayerWins;
      case Board::State::Zero_Wins:
        return tournament::GameOutcome::SecondPlayerWins;
      case Board::State::Draw:
        return tournament::GameOutcome::Draw;
      default:
        FATAL("Unexpected outcome");
    }

    std::swap(current_player, next_player);
  }
}

tournament::GameOutcome TicTacToeRules::play(const darwin::Genotype* x_genotype,
                                             const darwin::Genotype* o_genotype) const {
  AnnPlayer x_player;
  x_player.grow(x_genotype);

  AnnPlayer o_player;
  o_player.grow(o_genotype);

  return play(&x_player, &o_player);
}

}  // namespace tic_tac_toe
