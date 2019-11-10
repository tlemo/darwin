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

#include <core_ui/properties_widget.h>
#include <third_party/box2d/box2d.h>

#include <QFrame>
#include <QIcon>
#include <QTimer>

#include <memory>
using namespace std;

namespace physics_ui {

namespace Ui {
class Box2dSandboxWindow;
}

class Box2dWidget;
class Box2dSceneUi;

//! A reusable sandbox window for domains based on Box2D physics simulations
class Box2dSandboxWindow : public QFrame {
  Q_OBJECT

  static constexpr int kDefaultTimerSpeed = 20;  // ms

  const QIcon kPlayIcon{ ":/resources/mc_play.png" };
  const QIcon kPauseIcon{ ":/resources/mc_pause.png" };

 public:
  //! Sandbox state
  enum class State { None, Running, Paused, Failed, Completed };

 public:
  //! Creates a new sandbox windows
  Box2dSandboxWindow();
  virtual ~Box2dSandboxWindow();

  void addBottomPane(QWidget* widget);

 protected:
  void focusInEvent(QFocusEvent* event) override;

  //! Current state
  State state() const { return state_; }

  //! User-friendly string describing the current state
  QString stateDescription() const;

  //! Called to create a new scene
  virtual void newScene() = 0;

  //! Called to advance the simulation one step
  virtual void singleStep() = 0;

  //! Called when the whole UI (ex. variable values) must be updated
  virtual void updateUI() = 0;

  //! Sets the associated Box2D world
  void setWorld(b2World* world, const QRectF& viewport);

  //! Sets a custom scene UI
  void setSceneUi(Box2dSceneUi* scene_ui);

  //! Play/resume simulation
  void play();

  //! Pause the simulation
  void pause();

  //! Terminate the simulation, with the given final state
  void stop(State state);

  //! The Box2D widget
  Box2dWidget* box2dWidget() const;

  //! The properties widget used to show sandbox values
  core_ui::PropertiesWidget* variablesWidget() const;

 private slots:
  void on_play_pause_clicked();
  void on_single_step_clicked();
  void on_restart_clicked();
  void on_simulation_speed_valueChanged(int value);

 private:
  Ui::Box2dSandboxWindow* ui = nullptr;
  QTimer timer_;
  State state_ = State::None;
};

}  // namespace physics_ui
