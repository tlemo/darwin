// Copyright 2019 The Darwin Neuroevolution Framework Authors.
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

#include "simple_tournament.h"

#include <core/evolution.h>
#include <core/parallel_for_each.h>
#include <core/random.h>

namespace tournament {

SimpleTournament::SimpleTournament(const core::PropertySet& config) {
  config_.copyFrom(config);
}

void SimpleTournament::evaluatePopulation(darwin::Population* population,
                                          GameRules* game_rules) {
  darwin::StageScope stage("Tournament", population->size());
  pp::for_each(*population, [&](int index, darwin::Genotype* genotype) {
    default_random_engine rnd(core::randomSeed());
    uniform_int_distribution<size_t> dist_opponent(0, population->size() - 1);

    float score = 0;
    int eval_games = 0;

    for (int i = 0; i < config_.eval_games; ++i) {
      // pick a random (but different) opponent
      size_t opponent_index = dist_opponent(rnd);
      while (opponent_index == size_t(index)) {
        opponent_index = dist_opponent(rnd);
      }

      auto opponent_genotype = population->genotype(opponent_index);

      auto outcome = game_rules->play(genotype, opponent_genotype);
      score += game_rules->scores(outcome).player1_score;
      ++eval_games;

      if (config_.rematches) {
        auto rematch_outcome = game_rules->play(opponent_genotype, genotype);
        score += game_rules->scores(rematch_outcome).player2_score;
        ++eval_games;
      }
    }

    // normalize the fitness to make it invariant to the number of played games
    genotype->fitness = score / eval_games;

    darwin::ProgressManager::reportProgress();
  });
}

}  // namespace tournament
