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

#include "game.h"
#include "player.h"
#include "ann_player.h"

#include <core/utils.h>

#include <assert.h>
#include <math.h>
#include <algorithm>
#include <random>
using namespace std;

namespace conquest {

Config g_config;

Game::Game(int max_steps, const Board* board) : max_steps_(max_steps), board_(board) {
  CHECK(board_ != nullptr);
  node_units_.resize(board->nodes.size());
  deployments_.resize(board->arcs.size());
}

void Game::resetGame() {
  CHECK(state_ != State::InProgress);

  state_ = State::InProgress;
  step_ = 0;

  for (auto& units : node_units_)
    units = 0;

  for (auto& deployment : deployments_)
    deployment = {};

  node_units_[blue_start_node_] = g_config.initial_units;
  node_units_[red_start_node_] = -g_config.initial_units;

  blue_player_->newGame(this, Player::Side::Blue);
  red_player_->newGame(this, Player::Side::Red);
}

void Game::newGame(Player* blue_player, Player* red_player) {
  blue_player_ = blue_player;
  red_player_ = red_player;

  // pick (random) start positions
  uniform_int_distribution<size_t> dist_start(0, board_->start_nodes.size() - 1);
  bernoulli_distribution dist_coin;
  auto start_nodes = board_->start_nodes[dist_start(rnd_)];
  blue_start_node_ = start_nodes.blue_node;
  red_start_node_ = start_nodes.red_node;
  if (dist_coin(rnd_))
    std::swap(blue_start_node_, red_start_node_);

  resetGame();
}

void Game::rematch() {
  swap(blue_start_node_, red_start_node_);
  resetGame();
}

bool Game::gameStep() {
  if (state_ != State::InProgress)
    return false;

  bool check_stats = false;

  // draw?
  assert(step_ >= 0);
  if (step_++ == max_steps_) {
    state_ = State::Draw;
    return false;
  }

  // blue player
  int blue_order = blue_player_->order();
  if (blue_order != Player::kNoAction) {
    assert(node_units_[board_->arcs[blue_order].src] > 0);
    order(blue_order);
  }

  // red player
  int red_order = red_player_->order();
  if (red_order != Player::kNoAction) {
    assert(node_units_[board_->arcs[red_order].src] < 0);
    order(red_order);
  }

  // update deployments
  for (size_t i = 0; i < deployments_.size(); ++i) {
    auto& deployment = deployments_[i];
    const auto& arc = board_->arcs[i];

    if (deployment.size == 0)
      continue;

    deployment.position += g_config.units_speed;
    if (deployment.position >= arc.length) {
      node_units_[arc.dst] += deployment.size;
      deployment = {};
      check_stats = true;
    }
  }

  // game over?
  if (check_stats) {
    auto stats = computeStats();

    if (stats.blue_units == 0 && stats.red_units == 0) {
      // unlikely
      assert(stats.blue_nodes == 0);
      assert(stats.red_nodes == 0);
      state_ = State::Draw;
    } else if (stats.blue_units == 0) {
      assert(stats.blue_nodes == 0);
      state_ = State::RedWins;
    } else if (stats.red_units == 0) {
      assert(stats.red_nodes == 0);
      state_ = State::BlueWins;
    }

    if (state_ != State::InProgress)
      return false;
  }

  // unit production
  for (float& node : node_units_) {
    if (node == 0)
      continue;

    float units = fabsf(node);
    if (units < g_config.production_cap) {
      units += g_config.production_step;
      if (units > g_config.production_cap)
        units = g_config.production_cap;
    }
    node = (node > 0) ? units : -units;
  }

  return true;
}

void Game::order(int arc_index) {
  CHECK(arc_index >= 0 && arc_index < int(deployments_.size()));

  auto& deployment = deployments_[arc_index];
  const auto& arc = board_->arcs[arc_index];

  // existing deployment on the same arc?
  if (deployment.size != 0)
    return;

  float& units = node_units_[arc.src];
  float size = units * g_config.deploy_percent;

  // round deployment size
  size = int(size / g_config.deploy_resolution) * g_config.deploy_resolution;

  if (fabsf(size) >= g_config.deploy_min) {
    units -= size;
    deployment.size = size;
    deployment.position = 0;
  }
}

bool Game::finished() const {
  switch (state_) {
    case State::BlueWins:
    case State::RedWins:
    case State::Draw:
      return true;

    default:
      return false;
  }
}

Game::Stats Game::computeStats() const {
  Stats s = {};

  for (float units : node_units_) {
    if (units > 0) {
      ++s.blue_nodes;
      s.blue_units += units;
    } else if (units < 0) {
      ++s.red_nodes;
      s.red_units += -units;
    }
  }

  for (const auto& deployment : deployments_) {
    if (deployment.size > 0) {
      ++s.blue_orders;
      s.blue_units += deployment.size;
    } else if (deployment.size < 0) {
      ++s.red_orders;
      s.red_units += -deployment.size;
    }
  }

  return s;
}

float Game::nodeUnits(int index) const {
  CHECK(index >= 0 && index < int(node_units_.size()));
  return node_units_[index];
}

const Game::Deployment& Game::deployment(int index) const {
  CHECK(index >= 0 && index < int(deployments_.size()));
  return deployments_[index];
}

tournament::Scores ConquestRules::scores(tournament::GameOutcome outcome) const {
  switch (outcome) {
    case tournament::GameOutcome::FirstPlayerWins:
      return { g_config.win_points, g_config.lose_points };
    case tournament::GameOutcome::Draw:
      return { g_config.draw_points, g_config.draw_points };
    case tournament::GameOutcome::SecondPlayerWins:
      return { g_config.lose_points, g_config.win_points };
    default:
      FATAL("unexpected outcome");
  }
}

tournament::GameOutcome ConquestRules::play(Player* player1, Player* player2) const {
  Game game(g_config.max_steps, board_);
  game.newGame(player1, player2);

  // play the game
  while (game.gameStep())
    ;

  switch (game.state()) {
    case Game::State::BlueWins:
      return tournament::GameOutcome::FirstPlayerWins;
    case Game::State::RedWins:
      return tournament::GameOutcome::SecondPlayerWins;
    case Game::State::Draw:
      return tournament::GameOutcome::Draw;
    default:
      FATAL("Unexpected game state");
  }
}

tournament::GameOutcome ConquestRules::play(const darwin::Genotype* player1,
                                            const darwin::Genotype* player2) const {
  AnnPlayer blue_player;
  blue_player.grow(player1);

  AnnPlayer red_player;
  red_player.grow(player2);
  
  return play(&blue_player, &red_player);
}

}  // namespace conquest
