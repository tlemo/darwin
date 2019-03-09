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
#include <core/stringify.h>
#include <core/tournament_implementations.h>

namespace tic_tac_toe {

//! Tic-Tac-Toe artificial brain type
enum class AnnType {
  Value,   //!< Value network (a single value per board configuration)
  Policy,  //!< Policy network (one output signal for each potential move)
};

inline auto customStringify(core::TypeTag<AnnType>) {
  static auto stringify = new core::StringifyKnownValues<AnnType>{
    { AnnType::Value, "value" },
    { AnnType::Policy, "policy" },
  };
  return stringify;
}

//! Tic-Tac-Toe domain configuration
struct Config : public core::PropertySet {
  PROPERTY(ann_type, AnnType, AnnType::Value, "The role of the evolved brains");
  PROPERTY(calibration_matches, int, 100, "Number of calibration games");

  VARIANT(tournament_type,
          tournament::TournamentVariant,
          tournament::TournamentType::Simple,
          "Tournament type");
};

extern Config g_config;

void init();

//! Domain: [Tic-tac-toe](https://en.wikipedia.org/wiki/Tic-tac-toe)
//!
//! Classic tic-tac-toe game on a 3x3 grid.
//!
//! ![](images/tic_tac_toe_sandbox.png)
//!
//! All the population members participate in a tournament and the outcomes are used to
//! calculate the individual fitness values.
//!
//! ### Inputs
//!
//! Input | Value
//! -----:|------
//!  0..8 | board square value (0 = empty, 1 = own, -1 = opponent)
//!
//! ### Outputs
//!
//! If Config::ann_type == AnnType::Value, then:
//!
//! Output | Value
//! ------:|------
//!      0 | preference signal for the input board configuration
//!
//! If Config::ann_type == AnnType::Policy, then:
//!
//! Output | Value
//! ------:|------
//!   0..8 | preference signal for selecting the corresponding move square
//!
class TicTacToe : public darwin::Domain {
 public:
  size_t inputs() const override;
  size_t outputs() const override;

  bool evaluatePopulation(darwin::Population* population) const override;

  unique_ptr<core::PropertySet> calibrateGenotype(
      const darwin::Genotype* genotype) const override;
};

class Factory : public darwin::DomainFactory {
  unique_ptr<darwin::Domain> create(const core::PropertySet& config) override {
    g_config.copyFrom(config);
    return make_unique<TicTacToe>();
  }

  unique_ptr<core::PropertySet> defaultConfig(
      darwin::ComplexityHint hint) const override {
    auto config = make_unique<Config>();
    switch (hint) {
      case darwin::ComplexityHint::Minimal:
        config->tournament_type.simple_tournament.eval_games = 2;
        config->tournament_type.simple_tournament.rematches = false;
        config->tournament_type.selectCase(tournament::TournamentType::Simple);
        config->calibration_matches = 3;
        break;

      case darwin::ComplexityHint::Balanced:
      case darwin::ComplexityHint::Extra:
        break;
    }
    return config;
  }
};

}  // namespace tic_tac_toe
