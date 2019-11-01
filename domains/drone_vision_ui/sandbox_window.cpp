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
#include <core_ui/physics/box2d_sandbox_dialog.h>

#include <QString>

namespace drone_vision_ui {

bool SandboxWindow::setup() {
  CHECK(!domain_);
  CHECK(!scene_);
  CHECK(!agent_);
  CHECK(!scene_ui_);
  CHECK(state() == State::None);

  auto snapshot = darwin::evolution()->snapshot();
  domain_ = dynamic_cast<const drone_vision::DroneVision*>(snapshot.domain);
  CHECK(domain_ != nullptr);

  const auto default_generation = snapshot.generation - 1;
  const auto default_max_steps = domain_->config().max_steps;

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
  CHECK(domain_ != nullptr);
  CHECK(max_steps_ > 0);

  setSceneUi(nullptr);
  scene_ui_.reset();

  const auto target_velocity = domain_->randomTargetVelocity();
  scene_ = make_unique<drone_vision::Scene>(target_velocity, domain_);
  agent_ = make_unique<drone_vision::Agent>(genotype_.get(), scene_.get());
  step_ = 0;

  const auto extents = scene_->extents();
  const QRectF viewport(
      extents.x, extents.y + extents.height, extents.width, -extents.height);
  setWorld(scene_->box2dWorld(), viewport);

  scene_ui_ = make_unique<SceneUi>(scene_.get());
  setSceneUi(scene_ui_.get());
}

void SandboxWindow::singleStep() {
  CHECK(scene_);
  CHECK(agent_);

  if (step_ < max_steps_) {
    agent_->simStep();
    if (!scene_->simStep()) {
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
#if 0
  const float wheel_distance = scene_->wheelDistance();
  const float wheel_velocity = scene_->wheelVelocity();
  const float pole_angle = math::radiansToDegrees(scene_->poleAngle());
  const float angular_velocity = math::radiansToDegrees(scene_->poleAngularVelocity());
  const float dist_from_target = fabs(wheel_distance - scene_->targetPosition());
  const float fitness_bonus = scene_->fitnessBonus() / max_steps_;

  variables_.state->setValue(stateDescription());
  variables_.step->setValue(step_);
  variables_.fitness_bonus->setValue(QString::asprintf("%.3f", fitness_bonus));
  variables_.position->setValue(QString::asprintf("%.3f", wheel_distance));
  variables_.velocity->setValue(QString::asprintf("%.3f", wheel_velocity));
  variables_.angle->setValue(QString::asprintf("%.2f", pole_angle));
  variables_.angular_velocity->setValue(QString::asprintf("%.3f", angular_velocity));
  variables_.dist_from_target->setValue(QString::asprintf("%.3f", dist_from_target));
#endif

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

}  // namespace drone_vision_ui
