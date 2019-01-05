// Copyright 2019 The Darwin Neuroevolution Framework Authors.
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

#include <domains/cart_pole/cart_pole.h>
#include <domains/cart_pole/world.h>
#include <domains/cart_pole/agent.h>
#include <core_ui/properties_widget.h>

#include <QFrame>
#include <QIcon>
#include <QTimer>

#include <memory>
using namespace std;

namespace cart_pole_ui {

namespace Ui {
class SandboxWindow;
}

class SandboxWindow : public QFrame {
  Q_OBJECT

  static constexpr int kDefaultTimerSpeed = 20;  // ms

  const QIcon kPlayIcon{ ":/resources/mc_play.png" };
  const QIcon kPauseIcon{ ":/resources/mc_pause.png" };

  struct Variables {
    // configuration
    core_ui::PropertyItem* generation = nullptr;
    core_ui::PropertyItem* max_steps = nullptr;

    // simulation state
    core_ui::PropertyItem* state = nullptr;
    core_ui::PropertyItem* step = nullptr;
    core_ui::PropertyItem* pos = nullptr;
    core_ui::PropertyItem* velocity = nullptr;
    core_ui::PropertyItem* angle = nullptr;
    core_ui::PropertyItem* angular_velocity = nullptr;
  };
  
  enum class State { None, Running, Paused, Failed, Completed };

 public:
  explicit SandboxWindow(QWidget* parent = nullptr);
  ~SandboxWindow();

  bool setup();

 private slots:
  void on_play_pause_clicked();
  void on_single_step_clicked();
  void on_restart_clicked();
  void on_simulation_speed_valueChanged(int value);

 private:
  void newEpisode();
  void singleStep();
  void updateUI();
  void setupVariables();
  void pause();
  void stop(State state);
  void play();

 private:
  Ui::SandboxWindow* ui = nullptr;

  Variables variables_;
  QTimer timer_;
  
  const cart_pole::CartPole* cart_pole_ = nullptr;

  shared_ptr<const darwin::Genotype> genotype_;
  unique_ptr<cart_pole::World> world_;
  unique_ptr<cart_pole::Agent> agent_;
  int step_ = -1;
  int max_steps_ = -1;
  
  State state_ = State::None;
};

}  // namespace cart_pole_ui
