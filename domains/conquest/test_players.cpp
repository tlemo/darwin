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
#include "board.h"
#include "game.h"

#include <algorithm>
#include <limits>
#include <random>
using namespace std;

namespace conquest {

int RandomPlayer::order() {
  uniform_int_distribution<int> dist_order(0, int(game_->board()->arcs.size() - 1));

  int order = dist_order(rnd_);
  const auto& arc = game_->board()->arcs[order];

  if (game_->nodeUnits(arc.src) * color_lens_ > 0)
    return order;

  return Player::kNoAction;
}

// search for the most effective attach (max attacking - defending units)
//
// TODO: also consider reinforcements
//
int HandcraftedPlayer::order() {
  const Board* board = game_->board();

  int best_order = Player::kNoAction;
  float max_delta = -numeric_limits<float>::infinity();

  for (int node_index = 0; node_index < board->nodes.size(); ++node_index) {
    float units = game_->nodeUnits(node_index) * color_lens_;

    // no point analyzing a position where we can't give valid orders
    if (units <= 0)
      continue;

    for (int arc_index = 0; arc_index < board->arcs.size(); ++arc_index) {
      const auto& arc = board->arcs[arc_index];
      if (arc.src != node_index || game_->deployment(arc_index).size != 0)
        continue;
      float target_units = game_->nodeUnits(arc.dst) * color_lens_;
      if (target_units <= 0 && units + target_units > max_delta) {
        max_delta = units + target_units;
        best_order = arc_index;
      }
    }
  }

  return best_order;
}

}  // namespace conquest
