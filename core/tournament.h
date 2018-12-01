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

#pragma once

#include <core/darwin.h>
#include <core/properties.h>

namespace tournament {

struct TournamentConfig : public core::PropertySet {
  PROPERTY(eval_games, int, 10, "Number of evaluation games");
  PROPERTY(rematches, bool, true, "Play both-side rematches?");
};

struct Scores {
  float player1_score = 0;
  float player2_score = 0;
};

enum class GameOutcome {
  FirstPlayerWins,
  SecondPlayerWins,
  Draw,
};

class GameRules : public core::NonCopyable {
 public:
  virtual ~GameRules() = default;
  
  virtual GameOutcome play(const darwin::Genotype* player1,
                           const darwin::Genotype* player2) const = 0;

  virtual Scores scores(GameOutcome outcome) const = 0;
};

class Tournament : public core::NonCopyable {
 public:
  Tournament(const core::PropertySet& config, GameRules* game_rules);

  void evaluatePopulation(darwin::Population* population);

 private:
  TournamentConfig config_;
  GameRules* game_rules_;
};

}  // namespace tournament
