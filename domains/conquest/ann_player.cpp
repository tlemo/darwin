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

#include "ann_player.h"
#include "board.h"
#include "game.h"

#include <core/format.h>

#include <assert.h>
#include <algorithm>
using namespace std;

namespace conquest {

size_t AnnPlayer::inputsCount(const Board* board) {
  return board->nodes.size() + board->arcs.size() * 2;
}

size_t AnnPlayer::outputsCount(const Board* board) {
  return board->arcs.size();
}

int AnnPlayer::order() {
  const Board* board = game_->board();

  // update inputs
  int input_index = 0;
  for (float units : game_->nodeUnits())
    brain->setInput(input_index++, units * color_lens_);
  for (const auto& deployment : game_->deployments()) {
    brain->setInput(input_index++, deployment.position);
    brain->setInput(input_index++, deployment.size * color_lens_);
  }

  brain->think();

  // pick the max output signal
  int max_index = 0;
  for (int i = 1; i < board->arcs.size(); ++i) {
    if (brain->output(i) > brain->output(max_index))
      max_index = i;
  }

  constexpr float kOutputThreshold = 0.1f;

  if (brain->output(max_index) > kOutputThreshold &&
      game_->nodeUnits(board->arcs[max_index].src) * color_lens_ > 0) {
    return max_index;
  }

  return Player::kNoAction;
}

string AnnPlayer::name() const {
  return core::format("ANN Gen %d", generation);
}

void AnnPlayer::grow(const darwin::Genotype* genotype) {
  assert(genotype != nullptr);
  brain = genotype->grow();
  this->genotype = genotype;
}

void AnnPlayer::newGame(const Game* game, Player::Side side) {
  assert(brain);
  assert(genotype != nullptr);
  Player::newGame(game, side);
  brain->resetState();
}

}  // namespace conquest
