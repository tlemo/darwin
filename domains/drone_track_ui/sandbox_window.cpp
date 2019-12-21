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
#include <QHBoxLayout>
#include <QGroupBox>
#include <QSizePolicy>

#include <random>
using namespace std;

namespace drone_track_ui {

bool SandboxWindow::setup() {
  CHECK(!domain_);
  CHECK(!scene_);
  CHECK(!agent_);
  CHECK(!scene_ui_);
  CHECK(state() == State::None);

  auto snapshot = darwin::evolution()->snapshot();
  domain_ = dynamic_cast<const drone_track::DroneTrack*>(snapshot.domain);
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

  setupUi();

  newScene();
  play();

  return true;
}

void SandboxWindow::newScene() {
  CHECK(domain_ != nullptr);
  CHECK(max_steps_ > 0);

  setSceneUi(nullptr);
  scene_ui_.reset();

  const auto& config = domain_->config();

  sim::TrackConfig track_config;
  track_config.width = config.track_width;
  track_config.complexity = config.track_complexity;
  track_config.resolution = config.track_resolution;
  track_config.area_width = drone_track::Scene::kWidth;
  track_config.area_height = drone_track::Scene::kHeight;
  track_config.curb_width = config.curb_width;
  track_config.curb_friction = config.curb_friction;
  track_config.gates = config.track_gates;
  track_config.solid_gate_posts = config.solid_gate_posts;
  const auto random_seed = std::random_device{}();
  track_ = make_unique<sim::Track>(random_seed, track_config);

  scene_ = make_unique<drone_track::Scene>(track_.get(), domain_);
  agent_ = make_unique<sim::DroneController>(genotype_.get(), scene_->drone());
  step_ = 0;

  const auto extents = scene_->extents();
  const QRectF viewport(
      extents.x, extents.y + extents.height, extents.width, -extents.height);

  setWorld(scene_->box2dWorld(), viewport);

  scene_ui_ = make_unique<SceneUi>(scene_.get());
  setSceneUi(scene_ui_.get());

  camera_widget_->setCamera(scene_->drone()->camera());
  compass_widget_->setSensor(scene_->drone()->compass());
  accelerometer_widget_->setSensor(scene_->drone()->accelerometer());
  touch_widget_->setSensor(scene_->drone()->touchSensor());
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

  if (scene_ui_) {
    scene_ui_->step();
  }

  updateUI();
}

void SandboxWindow::updateUI() {
  updateVariables();
  update();
}

void SandboxWindow::setupUi() {
  auto sensors_pane = new QFrame(this);
  sensors_pane->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  sensors_pane->setMinimumSize(256, 82);

  auto layout = new QHBoxLayout(sensors_pane);
  layout->setContentsMargins(0, 0, 0, 0);

  auto addSensorPane = [&](const char* title, QWidget* widget, int stretch_factor) {
    auto frame = new QGroupBox(title, sensors_pane);
    frame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    auto frame_layout = new QHBoxLayout(frame);
    frame_layout->setContentsMargins(4, 0, 4, 4);
    frame_layout->addWidget(widget);
    layout->addWidget(frame);
    layout->setStretchFactor(frame, stretch_factor);
  };

  camera_widget_ = new physics_ui::CameraWidget(this);
  camera_widget_->setMinimumSize(64, 64);
  addSensorPane("Camera", camera_widget_, 10);

  compass_widget_ = new physics_ui::CompassWidget(this);
  compass_widget_->setMinimumSize(64, 64);
  addSensorPane("Compass", compass_widget_, 1);

  touch_widget_ = new physics_ui::TouchSensorWidget(this);
  touch_widget_->setMinimumSize(64, 64);
  addSensorPane("Touch", touch_widget_, 1);

  accelerometer_widget_ = new physics_ui::AccelerometerWidget(this);
  accelerometer_widget_->setMinimumSize(64, 64);
  addSensorPane("Accelerometer", accelerometer_widget_, 1);

  addBottomPane(sensors_pane);
}

void SandboxWindow::setupVariables() {
  auto config_section = variablesWidget()->addSection("Configuration");
  variables_.generation = config_section->addProperty("Generation");
  variables_.max_steps = config_section->addProperty("Max steps");

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

}  // namespace drone_track_ui
