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

#include <core/properties.h>
#include <core/tournament.h>

#include <random>
#include <string>
#include <vector>
using namespace std;

namespace conquest {

class Player;

//! Conquest domain configuration
struct Config : public core::PropertySet {
  PROPERTY(calibration_matches, int, 100, "Number of calibration matches");

  PROPERTY(board,
           BoardConfiguration,
           BoardConfiguration::Hexagon,
           "Board configuration name");

  PROPERTY(max_steps, int, 2500, "If no one wins before max_steps, the game is a tie");

  PROPERTY(points_win, float, 1.0f, "Points for a win");
  PROPERTY(points_lose, float, 0.0f, "Points for a lost game");
  PROPERTY(points_draw, float, 0.4f, "Points for a draw");

  PROPERTY(int_unit_scale, float, 10.0f, "A scaling factor to display units as integers");

  // units parameters
  PROPERTY(initial_units, float, 0.1f, "Initial units");
  PROPERTY(production_cap, float, 1.05f, "Production cap per node");
  PROPERTY(production_step, float, 0.005f, "Production step");

  // order (attack) parameters
  PROPERTY(deploy_min,
           float,
           1.0f / int_unit_scale,
           "Minimum units required to deploy an attack");
  PROPERTY(deploy_resolution, float, deploy_min, "Deployment resolution");
  PROPERTY(units_speed, float, 2.0f, "Units move speed");
  PROPERTY(deploy_percent, float, 0.99f, "What % of units are deployed? (0..1]");

  VARIANT(tournament_type,
          tournament::TournamentVariant,
          tournament::TournamentType::Default,
          "Tournament type");
};

extern Config g_config;

class Game : public core::NonCopyable {
 public:
  enum class State { None, InProgress, BlueWins, RedWins, Draw };

  struct Stats {
    // abs(total units, including in-flight deployments)
    float blue_units;
    float red_units;

    // controlled nodes
    int blue_nodes;
    int red_nodes;

    // in flight deployments
    int blue_orders;
    int red_orders;
  };

  struct Deployment {
    float size;
    float position;
  };

 public:
  Game(int max_steps, const Board* board);

  void newGame(Player* blue_player, Player* red_player);
  void rematch();
  bool gameStep();
  void order(int arc_index);

  const Board* board() const { return board_; }
  const Player* bluePlayer() const { return blue_player_; }
  const Player* redPlayer() const { return red_player_; }

  State state() const { return state_; }
  int currentStep() const { return step_; }
  bool finished() const;

  Stats computeStats() const;

  float nodeUnits(int index) const;
  const Deployment& deployment(int index) const;

  const vector<float>& nodeUnits() const { return node_units_; }
  const vector<Deployment>& deployments() const { return deployments_; }

 private:
  void resetGame();

 private:
  State state_ = State::None;

  vector<float> node_units_;
  vector<Deployment> deployments_;

  int step_ = -1;
  const int max_steps_ = -1;

  Player* blue_player_ = nullptr;
  Player* red_player_ = nullptr;

  int blue_start_node_ = -1;
  int red_start_node_ = -1;

  default_random_engine rnd_{ random_device{}() };

  const Board* const board_ = nullptr;
};

class ConquestRules : public tournament::GameRules {
 public:
  explicit ConquestRules(const Board* board) : board_(board) {}

  tournament::Scores scores(tournament::GameOutcome outcome) const override;

  tournament::GameOutcome play(Player* player1, Player* player2) const;
  
  tournament::GameOutcome play(const darwin::Genotype* player1,
                               const darwin::Genotype* player2) const override;

 private:
  const Board* board_ = nullptr;
};

}  // namespace conquest
