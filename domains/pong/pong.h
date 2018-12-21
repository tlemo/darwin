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

#include "ann_player.h"
#include "game.h"

#include <core/darwin.h>

namespace pong {

void init();

//! Domain: The classic [Pong](https://en.wikipedia.org/wiki/Pong) game
//!
//! This is an implementation of the classic 2-player Pong game: each player controlls a
//! paddle (which can be moved up or down). The ball bounces off the walls, and if it goes
//! behind the paddle line, the other player scores a point.
//!
//! ![](images/pong_sandbox.png)
//!
//! All the population members participate in a tournament and the outcomes are used to
//! calculate the individual fitness values.
//!
//! ### Inputs
//!
//! Input | Value
//! -----:|------
//!     0 | my_paddle_pos
//!     1 | opponent_paddle_pos
//!     2 | ball_x
//!     3 | ball_y
//!     4 | ball_vx
//!     5 | ball_vy
//!
//! ### Outputs
//!
//! Output | Value
//! ------:|------
//!      0 | move_up
//!      1 | move_down
//!
//! The largest output signal determines the player action at the current step. If none
//! of the output values is larger than 0, then the default action is "do nothing".
//!
class Pong : public darwin::Domain {
 public:
  size_t inputs() const override { return AnnPlayer::kInputs; }
  size_t outputs() const override { return AnnPlayer::kOutputs; }

  bool evaluatePopulation(darwin::Population* population) const override;

  unique_ptr<core::PropertySet> calibrateGenotype(
      const darwin::Genotype* genotype) const override;
};

class Factory : public darwin::DomainFactory {
  unique_ptr<darwin::Domain> create(const core::PropertySet& config) override;

  unique_ptr<core::PropertySet> defaultConfig(darwin::ComplexityHint hint) const override;
};

}  // namespace pong
