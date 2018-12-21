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

#include "board.h"
#include "game.h"

#include <core/darwin.h>

namespace conquest {

void init();

//! Domain: Conquest, a two-player strategy game
//!
//! Conquest is an minimalistic real-time strategy game. It’s a two-player game, and the
//! "board" is a graph of connected nodes. A node can be captured by "attacking" with more
//! units than the “defending” units controlling the node. A controlled node automatically
//! generates new units at a fixed rate (up to a specified cap, ex. 10 units). The goal
//! of the game is to capture all the nodes.
//!
//! ![](images/conquest_sandbox.png)
//!
//! All the population members participate in a tournament and the outcomes are used to
//! calculate the individual fitness values.
//!
//! ### Inputs
//!
//! Input       | Value
//! -----------:|------
//!  [N]        | the number of units in each node
//!  N + 2i     | deployment[i].position
//!  N + 2i + 1 | deployment[i].size
//!
//! Where:
//! - N is the number of nodes in the board graph
//! - i = [0..A), A = number of directed arcs in the graph
//! - Deployment position (distance from the source node)
//! - The number of units and deployment sizes are negative if they are opponent's units
//!
//! ### Outputs
//!
//! Output | Value
//! ------:|------
//!    [A] | preference for initiating an attack on the corresponding arc
//!
//! The largest output signal, if greater than `kOutputThreshold`, indicates the intention
//! to initiate an attack on the corresponding arch. The attack order is valid only if the
//! source node is controlled by the player. An attack sends a configurable percentage of
//! the units (`g_config.deploy_percent`) along the selected arc.
//!
class Conquest : public darwin::Domain {
 public:
  Conquest();

  size_t inputs() const override { return inputs_; }
  size_t outputs() const override { return outputs_; }

  bool evaluatePopulation(darwin::Population* population) const override;

  unique_ptr<core::PropertySet> calibrateGenotype(
      const darwin::Genotype* genotype) const override;

 private:
  const Board* board_ = nullptr;
  size_t inputs_ = 0;
  size_t outputs_ = 0;
};

class Factory : public darwin::DomainFactory {
  unique_ptr<darwin::Domain> create(const core::PropertySet& config) override;

  unique_ptr<core::PropertySet> defaultConfig(darwin::ComplexityHint hint) const override;
};

}  // namespace conquest
