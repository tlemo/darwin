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

#include "conquest.h"
#include "ann_player.h"
#include "game.h"
#include "test_players.h"

#include <core/utils.h>
#include <core/darwin.h>
#include <core/evolution.h>
#include <core/logging.h>
#include <core/parallel_for_each.h>

#include <atomic>
#include <random>
using namespace std;

using core::log;

namespace conquest {

void init() {
  darwin::registry()->domains.add<Factory>("conquest");
}

Conquest::Conquest() {
  board_ = Board::getBoard(g_config.board);
  CHECK(board_ != nullptr);

  inputs_ = AnnPlayer::inputsCount(board_);
  outputs_ = AnnPlayer::outputsCount(board_);
}

// return the points for the blue player, considering the game outcome
static float gamePoints(Game::State outcome, int games_count) {
  float points = 0;
  switch (outcome) {
    case Game::State::BlueWins:
      points = g_config.win_points;
      break;
    case Game::State::RedWins:
      points = g_config.lose_points;
      break;
    case Game::State::Draw:
      points = g_config.draw_points;
      break;
    default:
      FATAL("unexpected game outcome");
  }

  // scale the fitness to make it invariant to the number of played games
  return points * 100.0f / games_count;
}

static Game::State playGame(Game& game) {
  while (game.gameStep())
    ;
  assert(game.finished());
  return game.state();
}

bool Conquest::evaluatePopulation(darwin::Population* population) const {
  darwin::StageScope stage("Evaluate population");

  const int generation = population->generation();
  log("\n. generation %d\n", generation);

  // "grow" players from the genotypes
  vector<AnnPlayer> players(population->size());

  {
    darwin::StageScope stage("Ontogenesis");
    pp::for_each(players, [&](int index, AnnPlayer& player) {
      auto genotype = population->genotype(index);
      player.grow(genotype);
      genotype->fitness = 0;
    });
  }

  // TODO: temporary, replace with a better/more generic stats collection
  atomic<int> total_count = 0;
  atomic<int> draws_count = 0;

  // let the tournament begin!
  //
  // TODO: stats (w/l/d)
  //
  {
    darwin::StageScope stage("Tournament", players.size());
    pp::for_each(players, [&](int player_index, AnnPlayer& player) {
      random_device rd;
      default_random_engine rnd(rd());
      uniform_int_distribution<size_t> dist(0, players.size() - 1);

      Game game(g_config.max_steps, board_);

      for (int i = 0; i < g_config.eval_games; ++i) {
        // CONSIDER: player.clone() instead of grow()
        AnnPlayer opponent;
        auto opponent_index = dist(rnd);
        opponent.grow(players[opponent_index].genotype);

        // play the game
        game.newGame(&player, &opponent);
        auto outcome = playGame(game);

        if (g_config.rematches) {
          game.rematch();
          if (outcome != playGame(game))
            outcome = Game::State::Draw;
        }

        ++total_count;
        if (outcome == Game::State::Draw)
          ++draws_count;

        population->genotype(player_index)->fitness +=
            gamePoints(outcome, g_config.eval_games);
      }

      darwin::ProgressManager::reportProgress();
    });
  }

  core::log("Total games: %d, draws: %d (%.2f%%)\n",
            total_count.load(),
            draws_count.load(),
            (double(draws_count) / total_count) * 100);

  return false;
}

struct CalibrationFitness : public core::PropertySet {
  PROPERTY(vs_random_orders, float, 0, "Score vs. a player choosing random orders");
  PROPERTY(vs_handcrafted, float, 0, "Score vs. a handcrafted player");
};

unique_ptr<core::PropertySet> Conquest::calibrateGenotype(
    const darwin::Genotype* genotype) const {
  darwin::StageScope stage("Evaluate champion");

  auto calibration = make_unique<CalibrationFitness>();

  Game game(g_config.max_steps, board_);

  AnnPlayer player;
  player.grow(genotype);

  // calibration: random player
  RandomPlayer random_player;
  for (int i = 0; i < g_config.calibration_games; ++i) {
    game.newGame(&player, &random_player);
    auto outcome = playGame(game);
    calibration->vs_random_orders += gamePoints(outcome, g_config.calibration_games);
  }

  // calibration: handcrafted
  HandcraftedPlayer handcrafted_player;
  for (int i = 0; i < g_config.calibration_games; ++i) {
    game.newGame(&player, &handcrafted_player);
    auto outcome = playGame(game);
    calibration->vs_handcrafted += gamePoints(outcome, g_config.calibration_games);
  }

  return calibration;
}

unique_ptr<darwin::Domain> Factory::create(const core::PropertySet& config) {
  g_config.copyFrom(config);
  return make_unique<Conquest>();
}

unique_ptr<core::PropertySet> Factory::defaultConfig(darwin::ComplexityHint hint) const {
  auto config = make_unique<Config>();
  switch (hint) {
    case darwin::ComplexityHint::Minimal:
      config->eval_games = 2;
      config->calibration_games = 2;
      config->max_steps = 1000;
      break;

    case darwin::ComplexityHint::Balanced:
      break;

    case darwin::ComplexityHint::Extra:
      config->eval_games = 10;
      config->calibration_games = 200;
      config->max_steps = 5000;
      break;
  }
  return config;
}

}  // namespace conquest
