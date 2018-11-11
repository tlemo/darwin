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

#include <core_ui/properties_widget.h>
#include <domains/pong/game.h>
#include <domains/pong/player.h>

#include <QFrame>

#include <memory>
using namespace std;

namespace pong_ui {

namespace Ui {
class GameWidget;
}

class GameWidget : public QFrame {
  Q_OBJECT

  struct Variables {
    core_ui::PropertyItem* left_player = nullptr;
    core_ui::PropertyItem* right_player = nullptr;
    core_ui::PropertyItem* current_set = nullptr;
    core_ui::PropertyItem* game_step = nullptr;
    core_ui::PropertyItem* score_left = nullptr;
    core_ui::PropertyItem* score_right = nullptr;
  };

 public:
  GameWidget();
  ~GameWidget();

  bool setup();

 private:
  void updateUI();
  void setupVariables();

  unique_ptr<pong::Player> newPlayer(const QString& type, int generation) const;

 private slots:
  void on_debug_rendering_toggled(bool checked);

 private:
  Ui::GameWidget* ui = nullptr;

  pong::Game game_;
  unique_ptr<pong::Player> left_player_;
  unique_ptr<pong::Player> right_player_;
  Variables variables_;
};

}  // namespace pong_ui
