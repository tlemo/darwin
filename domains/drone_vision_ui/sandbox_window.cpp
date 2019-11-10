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

  // add a camera widget
  camera_widget_ = new physics_ui::CameraWidget(this);
  camera_widget_->setMinimumSize(64, 64);
  camera_widget_->setMaximumSize(4096, 64);
  addBottomPane(camera_widget_);

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
  agent_ = make_unique<sim::DroneController>(genotype_.get(), scene_->drone());
  step_ = 0;

  variables_.initial_target_velocity->setValue(
      QString::asprintf("(%.2f, %.2f)", target_velocity.x, target_velocity.y));

  const auto extents = scene_->extents();
  const QRectF viewport(
      extents.x, extents.y + extents.height, extents.width, -extents.height);

  setWorld(scene_->box2dWorld(), viewport);

  scene_ui_ = make_unique<SceneUi>(scene_.get());
  setSceneUi(scene_ui_.get());

  camera_widget_->setCamera(scene_->drone()->camera());
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
  updateVariables();
  update();
}

void SandboxWindow::setupVariables() {
  auto config_section = variablesWidget()->addSection("Configuration");
  variables_.generation = config_section->addProperty("Generation");
  variables_.max_steps = config_section->addProperty("Max steps");
  variables_.initial_target_velocity =
      config_section->addProperty("Initial target velocity");

  auto game_state_section = variablesWidget()->addSection("Game state");
  variables_.state = game_state_section->addProperty("State");
  variables_.step = game_state_section->addProperty("Simulation step");
  variables_.fitness = game_state_section->addProperty("Current fitness value");
}

// this assumes that the set of variables is the same between scene instances
void SandboxWindow::setupSceneVariables() {
  CHECK(scene_variables_map_.empty());
  auto scene_variables_section = variablesWidget()->addSection("Scene");
  auto scene_variables = scene_->variables();
  for (const auto& var : scene_variables->properties()) {
    scene_variables_map_[var->name()] = scene_variables_section->addProperty(var->name());
  }
}

void SandboxWindow::updateVariables() {
  if (scene_ != nullptr) {
    const float fitness = scene_->fitness() / step_;
    variables_.state->setValue(stateDescription());
    variables_.step->setValue(step_);
    variables_.fitness->setValue(QString::asprintf("%.3f", fitness));

    if (scene_variables_map_.empty()) {
      setupSceneVariables();
    }

    auto scene_variables = scene_->variables();
    for (const auto& var : scene_variables->properties()) {
      auto property_item = scene_variables_map_.at(var->name());
      property_item->setValue(var->value());
    }
  }
}

}  // namespace drone_vision_ui
