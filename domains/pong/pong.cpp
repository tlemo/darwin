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
#include <core/exception.h>

#include <random>
using namespace std;

namespace pong {

void init() {
  darwin::registry()->domains.add<Factory>("pong");
}

bool Pong::evaluatePopulation(darwin::Population* population) const {
  darwin::StageScope stage("Evaluate population");

  const int generation = population->generation();
  core::log("\n. generation %d\n", generation);

  // currently there's only one type of tournament
  CHECK(g_config.tournament_type.tag() == tournament::TournamentType::Default);

  PongRules rules;
  tournament::Tournament tournament(g_config.tournament_type.default_tournament, &rules);
  tournament.evaluatePopulation(population);

  return false;
}

struct CalibrationFitness : public core::PropertySet {
  PROPERTY(vs_handcrafted, float, 0, "Score vs. a handcrafted player");
};

static float calibrationScore(const PongRules& rules,
                              Player& subject_player,
                              Player& calibration_player) {
  float calibration_score = 0;
  int calibration_games = 0;

  for (int i = 0; i < g_config.calibration_games; ++i) {
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

unique_ptr<core::PropertySet> Pong::calibrateGenotype(
    const darwin::Genotype* genotype) const {
  darwin::StageScope stage("Evaluate champion");

  PongRules rules;
  auto calibration = make_unique<CalibrationFitness>();

  AnnPlayer champion;
  champion.grow(genotype);

  // calibration: handcrafted
  HandcraftedPlayer handcrafted_player;
  calibration->vs_handcrafted = calibrationScore(rules, champion, handcrafted_player);

  return calibration;
}

unique_ptr<darwin::Domain> Factory::create(const core::PropertySet& config) {
  g_config.copyFrom(config);

  // config values validation
  if (g_config.sets_per_game < 1)
    throw core::Exception("Invalid config value: sets_per_game");
  if (g_config.sets_required_to_win <= g_config.sets_per_game / 2)
    throw core::Exception(
        "Invalid config values: sets_required_to_win <= sets_per_game / 2");
  if (g_config.sets_required_to_win > g_config.sets_per_game)
    throw core::Exception("Invalid config values: sets_required_to_win > sets_per_game");

  return make_unique<Pong>();
}

unique_ptr<core::PropertySet> Factory::defaultConfig(darwin::ComplexityHint hint) const {
  auto config = make_unique<Config>();
  switch (hint) {
    case darwin::ComplexityHint::Minimal:
      config->tournament_type.default_tournament.eval_games = 2;
      config->tournament_type.default_tournament.rematches = false;
      config->tournament_type.selectCase(tournament::TournamentType::Default);
      config->calibration_games = 3;
      config->max_steps = 1000;
      config->sets_per_game = 2;
      config->sets_required_to_win = 2;
      break;

    case darwin::ComplexityHint::Balanced:
      config->tournament_type.default_tournament.rematches = false;
      break;

    case darwin::ComplexityHint::Extra:
      config->max_steps = 10000;
      config->calibration_games = 250;
      break;
  }
  return config;
}

}  // namespace pong
