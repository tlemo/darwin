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
#include <core/utils.h>
#include <core/properties.h>

namespace tournament {

//! Final game scores
//! \sa GameRules
struct Scores {
  float player1_score = 0;  //!< Player1 score
  float player2_score = 0;  //!< Player2 score
};

//! Game outcome
//! \sa GameRules
enum class GameOutcome {
  FirstPlayerWins,   //!< First player wins
  SecondPlayerWins,  //!< Second player wins
  Draw,              //!< Game ended up in a draw
};

//! Game rules abstraction (used to run the tournament)
//! \sa Tournament
class GameRules : public core::NonCopyable {
 public:
  virtual ~GameRules() = default;

  //! Sets up a game between players "grown" from the argument genotypes  
  virtual GameOutcome play(const darwin::Genotype* player1,
                           const darwin::Genotype* player2) const = 0;

  //! Returns the final scores based on a game outcome
  virtual Scores scores(GameOutcome outcome) const = 0;
};

//! Tournament interface
class Tournament : public core::NonCopyable {
 public:
  //! Run the tournament and assigns fitness values based on the results
  virtual void evaluatePopulation(darwin::Population* population,
                                  GameRules* game_rules) = 0;
};

}  // namespace tournament
