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
#include "game.h"

#include <core/format.h>

#include <assert.h>
using namespace std;

namespace pong {

Player::Action AnnPlayer::action() {
  if (side_ == Side::Left) {
    brain->setInput(kInputMyPaddlePos, game_->paddlePosP1());
    brain->setInput(kInputOpponentPaddlePos, game_->paddlePosP2());
    brain->setInput(kInputBallX, game_->ball().x);
    brain->setInput(kInputBallY, game_->ball().y);
    brain->setInput(kInputBallVx, game_->ball().vx);
    brain->setInput(kInputBallVy, game_->ball().vy);
  } else {
    brain->setInput(kInputMyPaddlePos, game_->paddlePosP2());
    brain->setInput(kInputOpponentPaddlePos, game_->paddlePosP1());
    brain->setInput(kInputBallX, -game_->ball().x);
    brain->setInput(kInputBallY, game_->ball().y);
    brain->setInput(kInputBallVx, -game_->ball().vx);
    brain->setInput(kInputBallVy, game_->ball().vy);
  }

  brain->think();

  float move_up = brain->output(kOutputMoveUp);
  float move_down = brain->output(kOutputMoveDown);

  // TODO: make activation threshold configurable?
  auto action = Action::None;
  float max_activation = 0;

  if (move_up > max_activation) {
    action = Action::MoveUp;
    max_activation = move_up;
  }

  if (move_down > max_activation) {
    action = Action::MoveDown;
    max_activation = move_down;
  }

  return action;
}

string AnnPlayer::name() const {
  return core::format("ANN Gen %d", generation);
}

void AnnPlayer::grow(const darwin::Genotype* genotype) {
  assert(genotype != nullptr);
  brain = genotype->grow();
  this->genotype = genotype;
  stats = {};
}

void AnnPlayer::newGame(const Game* game, Side side) {
  assert(brain);
  assert(genotype != nullptr);
  Player::newGame(game, side);
  brain->resetState();
}

}  // namespace pong
