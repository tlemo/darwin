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
#include "player.h"

#include <core/darwin.h>

#include <memory>
using namespace std;

namespace tic_tac_toe {

class AnnPlayer : public Player {
 public:
  static int inputs() { return 9; }
  static int outputs();

  void grow(const darwin::Genotype* genotype, int generation = -1);

  auto genotype() const { return genotype_; }

  // Player interface
  void newGame(const Board* board, Board::Piece side) override;
  int move() override;

  string name() const override;

 private:
  int policyBrainMove();
  int valueBrainMove();

 private:
  unique_ptr<darwin::Brain> brain_;
  const darwin::Genotype* genotype_ = nullptr;
  int generation_ = -1;
};

}  // namespace tic_tac_toe
