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

#include "swiss_tournament.h"

#include <core/exception.h>
#include <core/evolution.h>
#include <core/parallel_for_each.h>
#include <core/logging.h>

#include <algorithm>
#include <random>
#include <vector>
using namespace std;

namespace tournament {

class SwissTournament::PairingLog {
 public:
  PairingLog(size_t population_size) : log_(population_size) {}

  // calling it with different players (p1', p2') pairs
  // doesn't result in a data race
  void recordPairing(int p1, int p2) {
    CHECK(p1 != p2);
    if (p1 > p2) {
      std::swap(p1, p2);
    }
    log_[p1].push_back(p2);
  }

  bool alreadyPaired(int p1, int p2) const {
    if (p1 > p2) {
      std::swap(p1, p2);
    }
    for (int p : log_[p1]) {
      if (p == p2) {
        return true;
      }
    }
    return false;
  }

 private:
  // log_[p1] is the list of p2(s), such that p1 < p2
  vector<vector<int>> log_;
};

SwissTournament::SwissTournament(const core::PropertySet& config) {
  config_.copyFrom(config);
}

void SwissTournament::evaluatePopulation(darwin::Population* population,
                                         GameRules* game_rules) {
  struct Pairing {
    int p1 = -1;
    int p2 = -1;
  };

  if (population->size() % 2 != 0)
    throw core::Exception("Swiss tournament requires an even population size");

  random_device rd;
  default_random_engine rnd(rd());

  // setup the index used to setup the pairings for each round
  vector<int> pairing_index(population->size());
  for (int i = 0; i < int(pairing_index.size()); ++i) {
    pairing_index[i] = i;
  }

  // reset fitness values
  for (size_t i = 0; i < population->size(); ++i) {
    population->genotype(i)->fitness = 0;
  }

  PairingLog pairing_log(population->size());

  darwin::StageScope stage("Tournament", config_.rounds);
  for (int round = 0; round < config_.rounds; ++round) {
    vector<Pairing> pairings;
    int unintentional_rematches = 0;

    {
      darwin::StageScope stage("Pairing");

      if (round == 0) {
        // random pairings for the first round
        std::shuffle(pairing_index.begin(), pairing_index.end(), rnd);
      } else {
        std::sort(pairing_index.begin(), pairing_index.end(), [&](int a, int b) {
          return population->genotype(a)->fitness > population->genotype(b)->fitness;
        });
      }

      for (size_t i = 0; i < pairing_index.size();) {
        const int p1 = pairing_index[i++];
        // find the first opponent which hasn't already been paired with p1
        bool found_new_opponent = false;
        for (size_t j = i; j < pairing_index.size(); ++j) {
          if (!pairing_log.alreadyPaired(p1, pairing_index[j])) {
            std::swap(pairing_index[i], pairing_index[j]);
            found_new_opponent = true;
            break;
          }
        }
        const int p2 = pairing_index[i++];

        // note that we setup the game in reverse order,
        // so p2 (ranked weaker) will make the first move
        pairings.push_back({ p2, p1 });

        if (found_new_opponent) {
          pairing_log.recordPairing(p2, p1);
        } else {
          ++unintentional_rematches;
        }
      }
    }

    core::log("Swiss round pairing: %d unintentional rematches\n",
              unintentional_rematches);

    {
      darwin::StageScope stage("Tournament round", pairings.size());
      pp::for_each(pairings, [&](int, const Pairing& pairing) {
        auto p1_genotype = population->genotype(pairing.p1);
        auto p2_genotype = population->genotype(pairing.p2);

        const float score_scale = 1.0f / (config_.rounds * (config_.rematches ? 2 : 1));

        auto outcome = game_rules->play(p1_genotype, p2_genotype);
        auto scores = game_rules->scores(outcome);
        p1_genotype->fitness += scores.player1_score * score_scale;
        p2_genotype->fitness += scores.player2_score * score_scale;

        if (config_.rematches) {
          std::swap(p1_genotype, p2_genotype);
          auto rematch_outcome = game_rules->play(p1_genotype, p2_genotype);
          auto rematch_scores = game_rules->scores(rematch_outcome);
          p1_genotype->fitness += rematch_scores.player1_score * score_scale;
          p2_genotype->fitness += rematch_scores.player2_score * score_scale;
        }

        darwin::ProgressManager::reportProgress();
      });
    }

    darwin::ProgressManager::reportProgress();
  }

  // collect and report stats on ties in the final scores
  int ties = 0;
  for (size_t i = 1; i < population->size(); ++i) {
    if (population->genotype(i)->fitness == population->genotype(i - 1)->fitness) {
      ++ties;
    }
  }
  core::log("Final scores: %d ties\n\n", ties);
}

}  // namespace tournament
