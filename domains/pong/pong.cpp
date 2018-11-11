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

#include "pong.h"
#include "ann_player.h"
#include "game.h"
#include "test_players.h"

#include <core/utils.h>
#include <core/darwin.h>
#include <core/evolution.h>
#include <core/logging.h>
#include <core/parallel_for_each.h>

#include <random>
using namespace std;

using core::log;

namespace pong {

void init() {
  darwin::registry()->domains.add<Factory>("pong");
}

bool Pong::evaluatePopulation(darwin::Population* population) const {
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

  // let the tournament begin!
  {
    darwin::StageScope stage("Tournament", players.size());
    pp::for_each(players, [&](int player_index, AnnPlayer& player) {
      random_device rd;
      default_random_engine rnd(rd());
      uniform_int_distribution<size_t> dist(0, players.size() - 1);

      Game game(g_config.max_steps);

      for (int i = 0; i < g_config.eval_games; ++i) {
        // CONSIDER: player.clone() instead of grow()
        AnnPlayer opponent;
        auto opponent_index = dist(rnd);
        opponent.grow(players[opponent_index].genotype);

        // play the game
        game.newGame(&player, &opponent);

        for (int set = 0; set < g_config.sets_per_game; ++set) {
          while (game.gameStep())
            ;
          game.newSet();
        }

        // TODO: need over a % of points for a win? (also evalChampion)
        int points = g_config.points_tie;
        if (game.scoreP1() > game.scoreP2())
          points = g_config.points_win;
        else if (game.scoreP1() < game.scoreP2())
          points = g_config.points_lose;

        // TODO: update and publish stats

        // scale the fitness to make it invariant to the number of played games
        points *= 100.0f / g_config.eval_games;

        population->genotype(player_index)->fitness += points;
      }

      darwin::ProgressManager::reportProgress();
    });
  }

  return false;
}

struct CalibrationFitness : public core::PropertySet {
  PROPERTY(vs_handcrafted, float, 0, "Score vs. a handcrafted player");
};

unique_ptr<core::PropertySet> Pong::calibrateGenotype(
    const darwin::Genotype* genotype) const {
  darwin::StageScope stage("Evaluate champion");

  auto calibration = make_unique<CalibrationFitness>();

  Game game(g_config.max_steps);

  AnnPlayer player;
  player.grow(genotype);

  HandcraftedPlayer handcrafted_player;
  for (int i = 0; i < g_config.calibration_games; ++i) {
    game.newGame(&player, &handcrafted_player);

    for (int set = 0; set < g_config.sets_per_game; ++set) {
      while (game.gameStep())
        ;
      game.newSet();
    }

    int points = g_config.points_tie;
    if (game.scoreP1() > game.scoreP2())
      points = g_config.points_win;
    else if (game.scoreP1() < game.scoreP2())
      points = g_config.points_lose;

    // scale the fitness to make it invariant to the number of played games
    points *= 100.0f / g_config.calibration_games;
    calibration->vs_handcrafted += points;
  }

  return calibration;
}

unique_ptr<darwin::Domain> Factory::create(const core::PropertySet& config) {
  g_config.copyFrom(config);
  return make_unique<Pong>();
}

unique_ptr<core::PropertySet> Factory::defaultConfig(darwin::ComplexityHint hint) const {
  auto config = make_unique<Config>();
  switch (hint) {
    case darwin::ComplexityHint::Minimal:
      config->max_steps = 1000;
      config->eval_games = 2;
      config->calibration_games = 3;
      config->sets_per_game = 2;
      break;

    case darwin::ComplexityHint::Balanced:
      break;

    case darwin::ComplexityHint::Extra:
      config->max_steps = 10000;
      config->calibration_games = 250;
      break;
  }
  return config;
}

}  // namespace pong
