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

#include "sandbox_window.h"
#include "new_sandbox_dialog.h"
#include "ui_sandbox_window.h"

#include <core/darwin.h>
#include <core/evolution.h>
#include <core/logging.h>
#include <core/math_2d.h>

#include <QString>

namespace cart_pole_ui {

SandboxWindow::SandboxWindow(QWidget* parent)
    : QFrame(parent), ui(new Ui::SandboxWindow) {
  ui->setupUi(this);

  connect(ui->world_widget,
          &WorldWidget::sigPlayPause,
          this,
          &SandboxWindow::on_play_pause_clicked);
}

SandboxWindow::~SandboxWindow() {
  timer_.stop();
  delete ui;
}

bool SandboxWindow::setup() {
  CHECK(!cart_pole_);
  CHECK(!world_);
  CHECK(!agent_);
  CHECK(state_ == State::None);

  NewSandboxDialog dlg;
  if (dlg.exec() != QDialog::Accepted)
    return false;

  auto generation = dlg.generation();
  max_steps_ = dlg.maxSteps();

  try {
    auto snapshot = darwin::evolution()->snapshot();
    auto generation_summary = snapshot.trace->generationSummary(generation);

    genotype_ = generation_summary.champion;

    cart_pole_ = dynamic_cast<const cart_pole::CartPole*>(snapshot.domain);
    CHECK(cart_pole_ != nullptr);
  } catch (const exception& e) {
    core::log(
        "Generation %d champion genotype is not available: %s\n", generation, e.what());
    return false;
  }

  newEpisode();

  setWindowTitle(QString::asprintf("Generation %d", generation));
  setupVariables();

  // configuration values
  variables_.generation->setValue(generation);
  variables_.max_steps->setValue(max_steps_);

  // setup animation timer
  CHECK(!timer_.isActive());
  connect(&timer_, &QTimer::timeout, this, &SandboxWindow::singleStep);
  timer_.setInterval(kDefaultTimerSpeed);
  ui->simulation_speed->setValue(timer_.interval());
  ui->play_pause->setIcon(kPlayIcon);

  play();

  return true;
}

void SandboxWindow::on_play_pause_clicked() {
  if (timer_.isActive())
    pause();
  else
    play();

  updateUI();
}

void SandboxWindow::on_single_step_clicked() {
  pause();
  singleStep();
}

void SandboxWindow::on_restart_clicked() {
  pause();
  newEpisode();
  play();

  updateUI();
}

void SandboxWindow::on_simulation_speed_valueChanged(int value) {
  timer_.setInterval(value);
  updateUI();
}

void SandboxWindow::newEpisode() {
  CHECK(cart_pole_ != nullptr);
  CHECK(max_steps_ > 0);

  world_ = make_unique<cart_pole::World>(cart_pole_->randomInitialAngle(), cart_pole_);
  agent_ = make_unique<cart_pole::Agent>(genotype_.get(), world_.get());
  step_ = 0;

  ui->play_pause->setDisabled(false);
  ui->single_step->setDisabled(false);
  ui->world_widget->setWorld(world_.get());
  ui->world_widget->setFocus();

  state_ = State::Paused;
}

void SandboxWindow::pause() {
  CHECK(world_);
  if (state_ == State::Running) {
    timer_.stop();
    ui->play_pause->setIcon(kPlayIcon);
    state_ = State::Paused;
  }
}

void SandboxWindow::stop(State state) {
  CHECK(world_);
  timer_.stop();
  ui->play_pause->setIcon(kPlayIcon);
  ui->play_pause->setDisabled(true);
  ui->single_step->setDisabled(true);
  state_ = state;
}

void SandboxWindow::play() {
  CHECK(world_);
  if (state_ == State::Paused) {
    timer_.start();
    ui->play_pause->setIcon(kPauseIcon);
    state_ = State::Running;
  }
}

void SandboxWindow::singleStep() {
  CHECK(world_);
  CHECK(agent_);

  if (step_ < max_steps_) {
    agent_->simStep();
    if (!world_->simStep()) {
      stop(State::Failed);
    } else {
      ++step_;
    }
  } else {
    stop(State::Completed);
  }

  updateUI();
}

void SandboxWindow::updateUI() {
  const float cart_distance = world_->cartDistance();
  const float cart_velocity = world_->cartVelocity();
  const float pole_angle = math::radiansToDegrees(world_->poleAngle());
  const float angular_velocity = math::radiansToDegrees(world_->poleAngularVelocity());

  variables_.step->setValue(step_);
  variables_.pos->setValue(QString::asprintf("%.3f", cart_distance));
  variables_.velocity->setValue(QString::asprintf("%.3f", cart_velocity));
  variables_.angle->setValue(QString::asprintf("%.2f", pole_angle));
  variables_.angular_velocity->setValue(QString::asprintf("%.3f", angular_velocity));

  switch (state_) {
    case State::Running: {
      Q_ASSERT(timer_.isActive());
      double fps = 1000.0 / timer_.interval();
      variables_.state->setValue(QString::asprintf("Running (%.1f fps)", fps));
    } break;
    case State::Paused:
      Q_ASSERT(!timer_.isActive());
      variables_.state->setValue("Paused");
      break;
    case State::Failed:
      Q_ASSERT(!timer_.isActive());
      variables_.state->setValue("Failed");
      break;
    case State::Completed:
      Q_ASSERT(!timer_.isActive());
      variables_.state->setValue("Completed");
      break;
    case State::None:
      Q_ASSERT(!timer_.isActive());
      variables_.state->setValue("None");
      break;
    default:
      FATAL("Unexpected state");
  }

  update();
}

void SandboxWindow::setupVariables() {
  auto config_section = ui->variables->addSection("Configuration");
  variables_.generation = config_section->addProperty("Generation");
  variables_.max_steps = config_section->addProperty("Max steps");

  auto game_state_section = ui->variables->addSection("Game state");
  variables_.state = game_state_section->addProperty("State");
  variables_.step = game_state_section->addProperty("Simulation step");
  variables_.pos = game_state_section->addProperty("Cart position");
  variables_.velocity = game_state_section->addProperty("Cart velocity");
  variables_.angle = game_state_section->addProperty("Pole angle");
  variables_.angular_velocity = game_state_section->addProperty("Angular velocity");
}

}  // namespace cart_pole_ui
