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

#include <core/darwin.h>
#include <core/evolution.h>
#include <core/logging.h>
#include <core/math_2d.h>
#include <core_ui/sim/box2d_sandbox_dialog.h>

#include <QString>

namespace cart_pole_ui {

bool SandboxWindow::setup() {
  CHECK(!cart_pole_);
  CHECK(!world_);
  CHECK(!agent_);
  CHECK(state() == State::None);

  auto snapshot = darwin::evolution()->snapshot();
  cart_pole_ = dynamic_cast<const cart_pole::CartPole*>(snapshot.domain);
  CHECK(cart_pole_ != nullptr);

  const auto default_generation = snapshot.generation - 1;
  const auto default_max_steps = cart_pole_->config().max_steps;

  physics_ui::Box2dSandboxDialog dlg(default_generation, default_max_steps);
  if (dlg.exec() != QDialog::Accepted) {
    return false;
  }
  
  const int generation = dlg.generation();
  max_steps_ = dlg.maxSteps();
  CHECK(max_steps_ > 0);

  try {
    auto generation_summary = snapshot.trace->generationSummary(generation);
    genotype_ = generation_summary.champion;
  } catch (const exception& e) {
    core::log(
        "Generation %d champion genotype is not available: %s\n", generation, e.what());
    return false;
  }
  
  setWindowTitle(QString::asprintf("Generation %d", generation));
  setupVariables();

  // configuration values (constant values)
  variables_.generation->setValue(generation);
  variables_.max_steps->setValue(max_steps_);

  newScene();
  play();

  return true;
}

void SandboxWindow::newScene() {
  CHECK(cart_pole_ != nullptr);
  CHECK(max_steps_ > 0);

  world_ = make_unique<cart_pole::World>(cart_pole_->randomInitialAngle(), cart_pole_);
  agent_ = make_unique<cart_pole::Agent>(genotype_.get(), world_.get());
  step_ = 0;

  // calculate viewport extents based on the configuration values
  const auto& config = cart_pole_->config();
  constexpr float kMargin = 0.5f;
  const auto half_width = fmax(config.max_distance + kMargin, config.pole_length);
  const auto height = config.pole_length + kMargin;
  QRectF viewport(-half_width, height, 2 * half_width, -height);

  setWorld(world_->box2dWorld(), viewport);
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

  variables_.state->setValue(stateDescription());
  variables_.step->setValue(step_);
  variables_.pos->setValue(QString::asprintf("%.3f", cart_distance));
  variables_.velocity->setValue(QString::asprintf("%.3f", cart_velocity));
  variables_.angle->setValue(QString::asprintf("%.2f", pole_angle));
  variables_.angular_velocity->setValue(QString::asprintf("%.3f", angular_velocity));

  update();
}

void SandboxWindow::setupVariables() {
  auto config_section = variablesWidget()->addSection("Configuration");
  variables_.generation = config_section->addProperty("Generation");
  variables_.max_steps = config_section->addProperty("Max steps");

  auto game_state_section = variablesWidget()->addSection("Game state");
  variables_.state = game_state_section->addProperty("State");
  variables_.step = game_state_section->addProperty("Simulation step");
  variables_.pos = game_state_section->addProperty("Cart position");
  variables_.velocity = game_state_section->addProperty("Cart velocity");
  variables_.angle = game_state_section->addProperty("Pole angle");
  variables_.angular_velocity = game_state_section->addProperty("Angular velocity");
}

}  // namespace cart_pole_ui
