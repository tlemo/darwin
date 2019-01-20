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

#include <atomic>
#include <random>
using namespace std;

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

bool Conquest::evaluatePopulation(darwin::Population* population) const {
  darwin::StageScope stage("Evaluate population");

  const int generation = population->generation();
  core::log("\n. generation %d\n", generation);

  // currently there's only one type of tournament
  CHECK(g_config.tournament_type.tag() == tournament::TournamentType::Default);

  ConquestRules rules(board_);
  tournament::Tournament tournament(g_config.tournament_type.default_tournament, &rules);
  tournament.evaluatePopulation(population);

  return false;
}

struct CalibrationFitness : public core::PropertySet {
  PROPERTY(vs_random_orders, float, 0, "Score vs. a player choosing random orders");
  PROPERTY(vs_handcrafted, float, 0, "Score vs. a handcrafted player");
};

static float calibrationScore(const ConquestRules& rules,
                              Player& subject_player,
                              Player& calibration_player) {
  float calibration_score = 0;
  int calibration_games = 0;

  for (int i = 0; i < g_config.calibration_matches; ++i) {
    auto outcome = rules.play(&subject_player, &calibration_player);
    calibration_score += rules.scores(outcome).player1_score;
    ++calibration_games;

    auto rematch_outcome = rules.play(&calibration_player, &subject_player);
    calibration_score += rules.scores(rematch_outcome).player2_score;
    ++calibration_games;
  }

  // normalize the fitness to make it invariant to the number of played games
  return calibration_score / calibration_games * 100;
}

unique_ptr<core::PropertySet> Conquest::calibrateGenotype(
    const darwin::Genotype* genotype) const {
  darwin::StageScope stage("Evaluate champion");

  ConquestRules rules(board_);
  auto calibration = make_unique<CalibrationFitness>();

  AnnPlayer champion;
  champion.grow(genotype);

  // calibration: random player
  RandomPlayer random_player;
  calibration->vs_random_orders = calibrationScore(rules, champion, random_player);

  // calibration: handcrafted
  HandcraftedPlayer handcrafted_player;
  calibration->vs_handcrafted = calibrationScore(rules, champion, handcrafted_player);

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
      config->tournament_type.default_tournament.eval_games = 2;
      config->tournament_type.default_tournament.rematches = true;
      config->tournament_type.selectCase(tournament::TournamentType::Default);
      config->calibration_matches = 2;
      config->max_steps = 100;
      break;

    case darwin::ComplexityHint::Balanced:
      break;

    case darwin::ComplexityHint::Extra:
      config->calibration_matches = 200;
      config->max_steps = 5000;
      break;
  }
  return config;
}

}  // namespace conquest
