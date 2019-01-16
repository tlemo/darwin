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
#include <core_ui/box2d_sandbox_dialog.h>

#include <QString>

namespace unicycle_ui {

bool SandboxWindow::setup() {
  CHECK(!domain_);
  CHECK(!world_);
  CHECK(!agent_);
  CHECK(state() == State::None);

  auto snapshot = darwin::evolution()->snapshot();
  domain_ = dynamic_cast<const unicycle::Unicycle*>(snapshot.domain);
  CHECK(domain_ != nullptr);

  const auto default_generation = snapshot.generation - 1;
  const auto default_max_steps = domain_->config().max_steps;

  core_ui::Box2dSandboxDialog dlg(default_generation, default_max_steps);
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
  CHECK(domain_ != nullptr);
  CHECK(max_steps_ > 0);

  const float initial_angle_1 = domain_->randomInitialAngle();
  const float initial_angle_2 = domain_->randomInitialAngle();
  world_ =
      make_unique<unicycle::World>(initial_angle_1, initial_angle_2, domain_);
  agent_ = make_unique<unicycle::Agent>(genotype_.get(), world_.get());
  step_ = 0;

  // calculate viewport extents based on the configuration values
  const auto& config = domain_->config();
  constexpr float kMargin = 0.5f;
  const auto max_length = fmax(config.pole_1_length, config.pole_2_length);
  const auto half_width = fmax(config.max_distance + kMargin, max_length);
  const auto height = max_length + kMargin;
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
  const float pole_angle_1 = math::radiansToDegrees(world_->pole1Angle());
  const float angular_velocity_1 = math::radiansToDegrees(world_->pole1AngularVelocity());
  const float pole_angle_2 = math::radiansToDegrees(world_->pole2Angle());
  const float angular_velocity_2 = math::radiansToDegrees(world_->pole2AngularVelocity());

  variables_.state->setValue(stateDescription());
  variables_.step->setValue(step_);
  variables_.pos->setValue(QString::asprintf("%.3f", cart_distance));
  variables_.velocity->setValue(QString::asprintf("%.3f", cart_velocity));
  variables_.angle_1->setValue(QString::asprintf("%.2f", pole_angle_1));
  variables_.angular_velocity_1->setValue(QString::asprintf("%.3f", angular_velocity_1));
  variables_.angle_2->setValue(QString::asprintf("%.2f", pole_angle_2));
  variables_.angular_velocity_2->setValue(QString::asprintf("%.3f", angular_velocity_2));

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
  variables_.angle_1 = game_state_section->addProperty("Pole 1 angle");
  variables_.angular_velocity_1 = game_state_section->addProperty("Angular velocity 1");
  variables_.angle_2 = game_state_section->addProperty("Pole 2 angle");
  variables_.angular_velocity_2 = game_state_section->addProperty("Angular velocity 2");
}

}  // namespace unicycle_ui
