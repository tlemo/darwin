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

#include "game.h"

#include <domains/tic_tac_toe/player.h>

#include <QFrame>

#include <memory>
using namespace std;

namespace tic_tac_toe_ui {

namespace Ui {
class SandboxWindow;
}

class SandboxWindow : public QFrame {
  Q_OBJECT

 public:
  explicit SandboxWindow();
  ~SandboxWindow();

  bool setup();

 private slots:
  void on_reset_clicked();
  void on_flip_sides_clicked();
  void on_first_move_clicked();
  void on_prev_move_clicked();
  void on_next_move_clicked();
  void on_last_move_clicked();

 private:
  void play();
  void updatePlayerNames();

 private:
  Ui::SandboxWindow* ui = nullptr;

  Game game_;
  unique_ptr<tic_tac_toe::Player> x_player_;
  unique_ptr<tic_tac_toe::Player> o_player_;
};

}  // namespace tic_tac_toe_ui
