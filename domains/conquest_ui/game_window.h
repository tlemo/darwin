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
#include <domains/conquest/game.h>
#include <domains/conquest/player.h>

#include <QFrame>
#include <QTimer>

#include <memory>
using namespace std;

namespace conquest_ui {

namespace Ui {
class GameWindow;
}

class GameWindow : public QFrame {
  Q_OBJECT

  struct Variables {
    core_ui::PropertyItem* blue_player = nullptr;
    core_ui::PropertyItem* red_player = nullptr;
    core_ui::PropertyItem* state = nullptr;
    core_ui::PropertyItem* blue_units = nullptr;
    core_ui::PropertyItem* red_units = nullptr;
    core_ui::PropertyItem* blue_nodes = nullptr;
    core_ui::PropertyItem* red_nodes = nullptr;
    core_ui::PropertyItem* game_step = nullptr;
  };

 public:
  explicit GameWindow();
  ~GameWindow();

  bool setup();

 private slots:
  void pauseGame();
  void resumeGame();
  void gameStep();

  void on_debug_rendering_toggled(bool checked);

 private:
  void updateUI();
  void setupVariables();

  unique_ptr<conquest::Player> newPlayer(const QString& type, int generation) const;

 private:
  Ui::GameWindow* ui = nullptr;
  QTimer timer_;

  const conquest::Board* board_ = nullptr;
  unique_ptr<conquest::Game> game_;
  unique_ptr<conquest::Player> blue_player_;
  unique_ptr<conquest::Player> red_player_;
  Variables variables_;
};

}  // namespace conquest_ui
