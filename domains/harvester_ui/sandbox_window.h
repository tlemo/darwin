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
#include <domains/harvester/robot.h>
#include <domains/harvester/world.h>
#include <domains/harvester/world_map.h>

#include <QFrame>
#include <QIcon>
#include <QTimer>

#include <memory>
using namespace std;

namespace harvester_ui {

namespace Ui {
class SandboxWindow;
}

class SandboxWindow : public QFrame {
  Q_OBJECT

  static constexpr int kDefaultTimerSpeed = 40;  // msec

  const QIcon kPlayIcon{ ":/resources/mc_play.png" };
  const QIcon kPauseIcon{ ":/resources/mc_pause.png" };

  struct Variables {
    // configuration
    core_ui::PropertyItem* generation = nullptr;
    core_ui::PropertyItem* initial_health = nullptr;
    core_ui::PropertyItem* world_width = nullptr;
    core_ui::PropertyItem* world_height = nullptr;

    // simulation state
    core_ui::PropertyItem* state = nullptr;
    core_ui::PropertyItem* sim_step = nullptr;
    core_ui::PropertyItem* health = nullptr;
    core_ui::PropertyItem* pos = nullptr;
    core_ui::PropertyItem* angle = nullptr;
    core_ui::PropertyItem* last_move_dist = nullptr;
    core_ui::PropertyItem* total_move_dist = nullptr;
    core_ui::PropertyItem* good_fruits = nullptr;
    core_ui::PropertyItem* junk_fruits = nullptr;
    core_ui::PropertyItem* bad_fruits = nullptr;
    core_ui::PropertyItem* visited_cells = nullptr;
  };

 public:
  explicit SandboxWindow(QWidget* parent = nullptr);
  ~SandboxWindow();

  bool setup();

 private slots:
  void on_simulation_speed_valueChanged(int value);
  void on_play_pause_clicked();
  void on_single_step_clicked();
  void on_restart_clicked();

 private:
  void singleStep();
  void updateUI();
  void setupVariables();
  void pause();
  void play();

 private:
  Ui::SandboxWindow* ui = nullptr;
  QTimer timer_;

  unique_ptr<harvester::Robot> robot_;
  unique_ptr<harvester::World> world_;
  Variables variables_;
};

}  // namespace harvester_ui
