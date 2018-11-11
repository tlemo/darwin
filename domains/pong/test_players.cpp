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

#include "test_players.h"
#include "game.h"

namespace pong {

Player::Action HandcraftedPlayer::action() {
  auto my_paddle_pos =
      (side_ == Side::Left) ? game_->paddlePosP1() : game_->paddlePosP2();

  const auto& ball = game_->ball();
  const float paddle_half_size = g_config.paddle_size / 2;

  // only track the ball if it's on its side of the court
  // (to discourage simple mirroring strategies)
  constexpr float track_range = 0.25f;
  if (side_ == Side::Left && ball.x > track_range)
    return Action::None;
  if (side_ == Side::Right && ball.x < -track_range)
    return Action::None;

  // attempt to track the ball
  if (ball.y > my_paddle_pos + paddle_half_size)
    return Action::MoveUp;
  else if (ball.y < my_paddle_pos - paddle_half_size)
    return Action::MoveDown;

  return Action::None;
}

}  // namespace pong
