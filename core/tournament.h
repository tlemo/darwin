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

//! Tournament configuration
struct TournamentConfig : public core::PropertySet {
  PROPERTY(eval_games, int, 10, "Number of evaluation games");
  PROPERTY(rematches, bool, true, "Play both-side rematches?");
};

//! Tournament type
enum class TournamentType {
  Default,  //!< The default tournament implementation
};

inline auto customStringify(core::TypeTag<TournamentType>) {
  static auto stringify = new core::StringifyKnownValues<TournamentType>{
    { TournamentType::Default, "default" },
  };
  return stringify;
}

//! Tournament configurations
struct TournamentVariant : public core::PropertySetVariant<TournamentType> {
  CASE(TournamentType::Default, default_tournament, TournamentConfig);
};

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

//! A simple tournament implementation
//! 
//! Every genotype in a population is paired with a fixed number of random
//! opponents (genotypes from the same population). The fitness of the genotype
//! is updated based on the aggregated results from all the games.
//! 
//! \note The opponent's fitness is not updated
//! 
class Tournament : public core::NonCopyable {
 public:
  //! Creates a new tournament based on the specified game rules
  Tournament(const core::PropertySet& config, GameRules* game_rules);

  //! Assigns fitness values based on the tournament results
  void evaluatePopulation(darwin::Population* population);

 private:
  TournamentConfig config_;
  GameRules* game_rules_;
};

}  // namespace tournament
