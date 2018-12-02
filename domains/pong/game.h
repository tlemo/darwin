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

#include <core/properties.h>
#include <core/tournament.h>

namespace pong {

class Player;

// Game rules
// 1. player 2 serves first
// 2. the player who won the last set serves
// 3. whoever reaches "targetScore" first wins the game
// 4. if no one scores within "max_steps" the set is a tie, each player scores one point

//! Pong domain configuration
struct Config : public core::PropertySet {
  PROPERTY(paddle_size, float, 0.1f, "(0, 1)");
  PROPERTY(paddle_speed, float, 0.02f, "(0, inf)");
  PROPERTY(paddle_offset, float, 0.04f, "Horizontal distance from the board edge");

  PROPERTY(ball_radius, float, 0.01f, "Ball size");
  PROPERTY(ball_speed, float, 0.04f, "Ball speed");

  PROPERTY(calibration_games, int, 100, "Number of calibration games");

  PROPERTY(max_steps, int, 2500, "If no one scores before max_steps, the game is a draw");
  PROPERTY(sets_per_game, int, 10, "Sets per game");
  PROPERTY(sets_required_to_win, int, 6, "The minimum number of sets required to win");
  PROPERTY(simple_serve, bool, false, "Fixed serving angle");

  PROPERTY(points_win, float, 1.0f, "Points for a win");
  PROPERTY(points_lose, float, 0, "Points for a lost game");
  PROPERTY(points_draw, float, 0.4f, "Points for a tie");

  VARIANT(tournament_type,
          tournament::TournamentVariant,
          tournament::TournamentType::Default,
          "Tournament type");
};

extern Config g_config;

class Game : public core::NonCopyable {
 public:
  struct Ball {
    float x = 0;
    float y = 0;
    float vx = 0;
    float vy = 0;
  };

 public:
  explicit Game(int max_steps) : max_steps_(max_steps) {}

  void reset();
  void newGame(Player* player1, Player* player2);
  void newSet();
  bool gameStep();

  int scoreP1() const { return score_p1_; }
  int scoreP2() const { return score_p2_; }

  int currentSet() const { return set_; }
  int currentStep() const { return step_; }

  float paddlePosP1() const { return paddle_pos_p1_; }
  float paddlePosP2() const { return paddle_pos_p2_; }

  const Ball& ball() const { return ball_; }

  const Player* player1() const { return player1_; }
  const Player* player2() const { return player2_; }

 private:
  bool moveBall(float dt);

 private:
  int set_ = -1;
  int step_ = -1;
  int max_steps_ = -1;

  // scores
  int score_p1_ = -1;
  int score_p2_ = -1;

  // paddles
  float paddle_pos_p1_ = 0;
  float paddle_pos_p2_ = 0;

  Ball ball_;

  Player* player1_ = nullptr;
  Player* player2_ = nullptr;
};

class PongRules : public tournament::GameRules {
 public:
  tournament::Scores scores(tournament::GameOutcome outcome) const override;

  tournament::GameOutcome play(Player* player1, Player* player2) const;
  
  tournament::GameOutcome play(const darwin::Genotype* player1_genotype,
                               const darwin::Genotype* player2_genotype) const override;
};

}  // namespace pong
