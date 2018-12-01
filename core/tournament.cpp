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

#include "tournament.h"

#include <core/evolution.h>
#include <core/parallel_for_each.h>

namespace tournament {

Tournament::Tournament(const core::PropertySet& config, GameRules* game_rules)
    : game_rules_(game_rules) {
  config_.copyFrom(config);
}

void Tournament::evaluatePopulation(darwin::Population* population) {
  darwin::StageScope stage("Tournament", population->size());
  pp::for_each(*population, [&](int, darwin::Genotype* genotype) {
    random_device rd;
    default_random_engine rnd(rd());
    uniform_int_distribution<size_t> dist(0, population->size() - 1);

    float score = 0;
    int eval_games = 0;

    for (int i = 0; i < config_.eval_games; ++i) {
      auto opponent_genotype = population->genotype(dist(rnd));

      auto outcome = game_rules_->play(genotype, opponent_genotype);
      score += game_rules_->scores(outcome).player1_score;
      ++eval_games;

      if (config_.rematches) {
        auto rematch_outcome = game_rules_->play(opponent_genotype, genotype);
        score += game_rules_->scores(rematch_outcome).player2_score;
        ++eval_games;
      }
    }
    
    // normalize the fitness to make it invariant to the number of played games
    genotype->fitness = score / eval_games * 100;

    darwin::ProgressManager::reportProgress();
  });
}

}  // namespace tournament
