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

#include "tic_tac_toe.h"
#include "ann_player.h"
#include "test_players.h"

#include <core/utils.h>
#include <core/darwin.h>
#include <core/evolution.h>
#include <core/logging.h>
#include <core/parallel_for_each.h>

#include <algorithm>
#include <memory>
#include <random>
using namespace std;

using core::log;

namespace tic_tac_toe {

Config g_config;

void init() {
  darwin::registry()->domains.add<Factory>("tic_tac_toe");
}

static float score(Board::Outcome outcome) {
  switch (outcome) {
    case Board::Outcome::X_Wins:
      return +5;
    case Board::Outcome::Draw:
      return +1;
    case Board::Outcome::Zero_Wins:
      return 0;
    default:
      FATAL("unexpected outcome");
  }
}

static Board::Outcome reverseOutcome(Board::Outcome outcome) {
  switch (outcome) {
    case Board::Outcome::Draw:
      return outcome;
    case Board::Outcome::X_Wins:
      return Board::Outcome::Zero_Wins;
    case Board::Outcome::Zero_Wins:
      return Board::Outcome::X_Wins;
    default:
      FATAL("unexpected outcome");
  }
}

size_t TicTacToe::inputs() const {
  return AnnPlayer::inputs();
}

size_t TicTacToe::outputs() const {
  return AnnPlayer::outputs();
}

// play a complete game and returns the final outcome
static Board::Outcome playGame(Player* x_player, Player* o_player) {
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

    auto outcome = board.outcome();
    if (outcome != Board::Outcome::Undecided)
      return outcome;

    std::swap(current_player, next_player);
  }
}

bool TicTacToe::evaluatePopulation(darwin::Population* population) const {
  darwin::StageScope stage("Evaluate population");

  const int generation = population->generation();
  log("\n. generation %d\n", generation);

  // "grow" players from the genotypes
  vector<AnnPlayer> players(population->size());

  {
    darwin::StageScope stage("Ontogenesis");
    pp::for_each(players, [&](int index, AnnPlayer& player) {
      auto genotype = population->genotype(index);
      player.grow(genotype, generation);
      genotype->fitness = 0;
    });
  }

  // let the tournament begin!
  {
    darwin::StageScope stage("Tournament", players.size());
    pp::for_each(players, [&](int player_index, AnnPlayer& player) {
      random_device rd;
      default_random_engine rnd(rd());
      uniform_int_distribution<size_t> dist(0, population->size() - 1);

      float& fitness = population->genotype(player_index)->fitness;

      // normalize the fitness to make it invariant to the number of played games
      const float score_norm =
          1.0f / (g_config.eval_games * (g_config.rematches ? 2 : 1));

      for (int i = 0; i < g_config.eval_games; ++i) {
        auto opponent_index = dist(rnd);
        AnnPlayer opponent;
        opponent.grow(players[opponent_index].genotype(), generation);

        auto outcome = playGame(&player, &opponent);
        fitness += score(outcome) * score_norm;

        if (g_config.rematches) {
          auto rematch_outcome = playGame(&opponent, &player);
          rematch_outcome = reverseOutcome(rematch_outcome);
          fitness += score(rematch_outcome) * score_norm;
        }
      }

      darwin::ProgressManager::reportProgress();
    });
  }

  return false;
}

struct CalibrationFitness : public core::PropertySet {
  PROPERTY(vs_random_player, float, 0, "Play against a player picking random moves");
  PROPERTY(vs_average_player, float, 0, "Play against an average player");
};

static float calibrationScore(Player& subject_player, Player& reference_player) {
  float calibraton_score = 0;
  int calibration_games = 0;
  for (int i = 0; i < g_config.calibration_games; ++i) {
    auto outcome = playGame(&subject_player, &reference_player);
    calibraton_score += score(outcome);
    ++calibration_games;

    if (g_config.rematches) {
      auto rematch_outcome = playGame(&reference_player, &subject_player);
      rematch_outcome = reverseOutcome(rematch_outcome);
      calibraton_score += score(rematch_outcome);
      ++calibration_games;
    }
  }

  // normalize the fitness to make it invariant to the number of played games
  return calibraton_score / calibration_games;
}

unique_ptr<core::PropertySet> TicTacToe::calibrateGenotype(
    const darwin::Genotype* genotype) const {
  darwin::StageScope stage("Calibrate fitness");

  auto calibration = make_unique<CalibrationFitness>();

  AnnPlayer champion;
  champion.grow(genotype);

  // calibration: a completely random player
  RandomPlayer random_player(false);
  calibration->vs_random_player += calibrationScore(champion, random_player);

  // calibration: an average player (preferring winning and blocking moves)
  RandomPlayer average_player(true);
  calibration->vs_average_player += calibrationScore(champion, average_player);

  return calibration;
}

}  // namespace tic_tac_toe
