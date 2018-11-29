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
#include "game.h"

#include <core/utils.h>
#include <core/darwin.h>
#include <core/evolution.h>
#include <core/logging.h>

#include <algorithm>
#include <memory>
#include <random>
using namespace std;

namespace tic_tac_toe {

Config g_config;

void init() {
  darwin::registry()->domains.add<Factory>("tic_tac_toe");
}

size_t TicTacToe::inputs() const {
  return AnnPlayer::inputs();
}

size_t TicTacToe::outputs() const {
  return AnnPlayer::outputs();
}

bool TicTacToe::evaluatePopulation(darwin::Population* population) const {
  darwin::StageScope stage("Evaluate population");

  const int generation = population->generation();
  core::log("\n. generation %d\n", generation);

  // currently there's only one type of tournament
  CHECK(g_config.tournament_type.tag() == TournamentType::Default);

  Game game;
  tournament::Tournament tournament(g_config.tournament_type.default_tournament, &game);
  tournament.evaluatePopulation(population);

  return false;
}

struct CalibrationFitness : public core::PropertySet {
  PROPERTY(vs_random_player, float, 0, "Play against a player picking random moves");
  PROPERTY(vs_average_player, float, 0, "Play against an average player");
};

static float calibrationScore(const Game& game,
                              Player& subject_player,
                              Player& reference_player) {
  float calibration_score = 0;
  int calibration_games = 0;

  for (int i = 0; i < g_config.calibration_matches; ++i) {
    auto outcome = game.play(&subject_player, &reference_player);
    calibration_score += game.score(outcome);
    ++calibration_games;

    auto rematch_outcome = game.play(&reference_player, &subject_player);
    calibration_score += game.score(reverseOutcome(rematch_outcome));
    ++calibration_games;
  }

  // normalize the fitness to make it invariant to the number of played games
  return calibration_score / calibration_games * 100;
}

unique_ptr<core::PropertySet> TicTacToe::calibrateGenotype(
    const darwin::Genotype* genotype) const {
  darwin::StageScope stage("Calibrate fitness");

  Game game;
  auto calibration = make_unique<CalibrationFitness>();

  AnnPlayer champion;
  champion.grow(genotype);

  // calibration: a completely random player
  RandomPlayer random_player(false);
  calibration->vs_random_player = calibrationScore(game, champion, random_player);

  // calibration: an average player (preferring winning and blocking moves)
  RandomPlayer average_player(true);
  calibration->vs_average_player = calibrationScore(game, champion, average_player);

  return calibration;
}

}  // namespace tic_tac_toe
