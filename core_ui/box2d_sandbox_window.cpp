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

#include "box2d_sandbox_window.h"
#include "box2d_widget.h"
#include "ui_box2d_sandbox_window.h"

#include <core/darwin.h>
#include <core/evolution.h>
#include <core/logging.h>
#include <core/math_2d.h>

#include <QString>

namespace core_ui {

Box2dSandboxWindow::Box2dSandboxWindow()
    : QFrame(nullptr), ui(new Ui::Box2dSandboxWindow) {
  ui->setupUi(this);

  connect(ui->world_widget,
          &Box2dWidget::sigPlayPause,
          this,
          &Box2dSandboxWindow::on_play_pause_clicked);

  setFocusPolicy(Qt::StrongFocus);
}

Box2dSandboxWindow::~Box2dSandboxWindow() {
  timer_.stop();
  delete ui;
}

void Box2dSandboxWindow::focusInEvent(QFocusEvent* /*event*/) {
  ui->world_widget->setFocus();
}

QString Box2dSandboxWindow::stateDescription() const {
  switch (state_) {
    case State::Running: {
      Q_ASSERT(timer_.isActive());
      const double fps = 1000.0 / timer_.interval();
      return QString::asprintf("Running (%.1f fps)", fps);
    }
    case State::Paused:
      Q_ASSERT(!timer_.isActive());
      return "Paused";
    case State::Failed:
      Q_ASSERT(!timer_.isActive());
      return "Failed";
    case State::Completed:
      Q_ASSERT(!timer_.isActive());
      return "Completed";
    case State::None:
      Q_ASSERT(!timer_.isActive());
      return "None";
    default:
      FATAL("Unexpected state");
  }
}

void Box2dSandboxWindow::setWorld(b2World* world, const QRectF& viewport) {
  if (state_ == State::None) {
    // setup animation timer
    CHECK(!timer_.isActive());
    connect(&timer_, &QTimer::timeout, this, &Box2dSandboxWindow::singleStep);
    timer_.setInterval(kDefaultTimerSpeed);
    ui->simulation_speed->setValue(timer_.interval());
  }
  timer_.stop();
  ui->play_pause->setIcon(kPlayIcon);
  ui->play_pause->setDisabled(false);
  ui->single_step->setDisabled(false);
  ui->world_widget->setWorld(world, viewport);
  ui->world_widget->setFocus();
  state_ = State::Paused;
}

void Box2dSandboxWindow::setSceneUi(Box2dSceneUi* scene_ui) {
  ui->world_widget->setSceneUi(scene_ui);
}

void Box2dSandboxWindow::play() {
  if (state_ == State::Paused) {
    timer_.start();
    ui->play_pause->setIcon(kPauseIcon);
    state_ = State::Running;
  }
}

void Box2dSandboxWindow::pause() {
  if (state_ == State::Running) {
    timer_.stop();
    ui->play_pause->setIcon(kPlayIcon);
    state_ = State::Paused;
  }
}

void Box2dSandboxWindow::stop(State state) {
  CHECK(state_ != State::None);
  CHECK(state == State::Failed || state == State::Completed);
  timer_.stop();
  ui->play_pause->setIcon(kPlayIcon);
  ui->play_pause->setDisabled(true);
  ui->single_step->setDisabled(true);
  state_ = state;
}

Box2dWidget* Box2dSandboxWindow::box2dWidget() const {
  return ui->world_widget;
}

PropertiesWidget* Box2dSandboxWindow::variablesWidget() const {
  return ui->variables;
}

void Box2dSandboxWindow::on_play_pause_clicked() {
  if (timer_.isActive())
    pause();
  else
    play();

  updateUI();
}

void Box2dSandboxWindow::on_single_step_clicked() {
  pause();
  singleStep();
}

void Box2dSandboxWindow::on_restart_clicked() {
  pause();
  newScene();
  play();

  updateUI();
}

void Box2dSandboxWindow::on_simulation_speed_valueChanged(int value) {
  timer_.setInterval(value);
  updateUI();
}

}  // namespace core_ui
