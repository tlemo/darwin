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

namespace ballistics_ui {

bool SandboxWindow::setup() {
  CHECK(!domain_);
  CHECK(!world_);
  CHECK(!agent_);
  CHECK(!scene_ui_);
  CHECK(state() == State::None);

  auto snapshot = darwin::evolution()->snapshot();
  domain_ = dynamic_cast<const ballistics::Ballistics*>(snapshot.domain);
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

  const float initial_angle = domain_->randomInitialAngle();
  const float target_position = domain_->randomTargetPosition();
  world_ = make_unique<ballistics::World>(initial_angle, target_position, domain_);
  agent_ = make_unique<ballistics::Agent>(genotype_.get(), world_.get());
  step_ = 0;

  variables_.initial_angle->setValue(QString::asprintf("%.2f", initial_angle));
  variables_.target_position->setValue(QString::asprintf("%.2f", target_position));

  // calculate viewport extents based on the configuration values
  const auto& config = domain_->config();
  constexpr float kMargin = 0.3f;
  const auto half_width = fmax(config.max_distance + kMargin, config.pole_length);
  const auto height = config.pole_length + config.wheel_radius + kMargin;
  QRectF viewport(-half_width, height, 2 * half_width, -height);

  setWorld(world_->box2dWorld(), viewport);
  
  scene_ui_ = make_unique<SceneUi>(world_.get());
  setSceneUi(scene_ui_.get());
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
  const float wheel_distance = world_->wheelDistance();
  const float wheel_velocity = world_->wheelVelocity();
  const float pole_angle = math::radiansToDegrees(world_->poleAngle());
  const float angular_velocity = math::radiansToDegrees(world_->poleAngularVelocity());
  const float dist_from_target = fabs(wheel_distance - world_->targetPosition());
  const float fitness_bonus = world_->fitnessBonus() / max_steps_;

  variables_.state->setValue(stateDescription());
  variables_.step->setValue(step_);
  variables_.fitness_bonus->setValue(QString::asprintf("%.3f", fitness_bonus));
  variables_.position->setValue(QString::asprintf("%.3f", wheel_distance));
  variables_.velocity->setValue(QString::asprintf("%.3f", wheel_velocity));
  variables_.angle->setValue(QString::asprintf("%.2f", pole_angle));
  variables_.angular_velocity->setValue(QString::asprintf("%.3f", angular_velocity));
  variables_.dist_from_target->setValue(QString::asprintf("%.3f", dist_from_target));

  update();
}

void SandboxWindow::setupVariables() {
  auto config_section = variablesWidget()->addSection("Configuration");
  variables_.generation = config_section->addProperty("Generation");
  variables_.max_steps = config_section->addProperty("Max steps");
  variables_.initial_angle = config_section->addProperty("Initial angle");
  variables_.target_position = config_section->addProperty("Target position");

  auto game_state_section = variablesWidget()->addSection("Game state");
  variables_.state = game_state_section->addProperty("State");
  variables_.step = game_state_section->addProperty("Simulation step");
  variables_.fitness_bonus = game_state_section->addProperty("Fitness bonus");
  variables_.position = game_state_section->addProperty("Wheel position");
  variables_.velocity = game_state_section->addProperty("Wheel velocity");
  variables_.angle = game_state_section->addProperty("Pole angle");
  variables_.angular_velocity = game_state_section->addProperty("Angular velocity");
  variables_.dist_from_target = game_state_section->addProperty("Distance from target");
}

}  // namespace ballistics_ui
