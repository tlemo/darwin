// Copyright 2018 The Darwin Neuroevolution Framework Authors.
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

#include <QString>

#include <cmath>
using namespace std;

namespace harvester_ui {

SandboxWindow::SandboxWindow(QWidget* parent)
    : QFrame(parent), ui(new Ui::SandboxWindow) {
  ui->setupUi(this);

  connect(ui->world_widget,
          &WorldWidget::sigSingleStep,
          this,
          &SandboxWindow::on_single_step_clicked);
}

SandboxWindow::~SandboxWindow() {
  delete ui;
}

bool SandboxWindow::setup() {
  CHECK(!robot_);
  CHECK(!world_);

  NewSandboxDialog dlg;
  if (dlg.exec() != QDialog::Accepted)
    return false;

  auto generation = dlg.generation();
  auto world_width = dlg.worldWidth();
  auto world_height = dlg.worldHeight();

  harvester::WorldMap world_map(world_height, world_width);
  if (!world_map.generate()) {
    core::log("Can't generate sandbox map");
    return false;
  }

  try {
    auto snapshot = darwin::evolution()->snapshot();
    auto generation_summary = snapshot.trace->generationSummary(generation);
    robot_ = make_unique<harvester::Robot>();
    robot_->grow(generation_summary.champion.get(), dlg.initialHealth());
  } catch (const exception& e) {
    core::log(
        "Generation %d champion genotype is not available: %s\n", generation, e.what());
    return false;
  }

  world_ = make_unique<harvester::World>(world_map, robot_.get());
  world_->simInit();

  ui->world_widget->setWorld(world_.get());

  setWindowTitle(
      QString::asprintf("Gen %d (%dx%d)", generation, world_width, world_height));

  setupVariables();

  variables_.generation->setValue(generation);
  variables_.initial_health->setValue(dlg.initialHealth());
  variables_.world_width->setValue(world_width);
  variables_.world_height->setValue(world_height);

  CHECK(!timer_.isActive());
  connect(&timer_, &QTimer::timeout, this, &SandboxWindow::singleStep);
  timer_.setInterval(kDefaultTimerSpeed);
  ui->simulation_speed->setValue(timer_.interval());
  ui->play_pause->setIcon(kPlayIcon);

  return true;
}

void SandboxWindow::on_simulation_speed_valueChanged(int value) {
  timer_.setInterval(value);
  updateUI();
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

  // generate a new world map
  const auto& old_world_map = world_->worldMap();
  harvester::WorldMap world_map(int(old_world_map.cells.rows),
                                int(old_world_map.cells.cols));
  CHECK(world_map.generate());

  world_ = make_unique<harvester::World>(world_map, robot_.get());
  world_->simInit();

  ui->world_widget->setWorld(world_.get());

  updateUI();
}

void SandboxWindow::pause() {
  CHECK(world_);
  timer_.stop();
  ui->play_pause->setIcon(kPlayIcon);
}

void SandboxWindow::play() {
  CHECK(world_);
  timer_.start();
  ui->play_pause->setIcon(kPauseIcon);
}

void SandboxWindow::singleStep() {
  if (robot_->alive())
    world_->simStep();
  updateUI();
}

void SandboxWindow::updateUI() {
  variables_.sim_step->setValue(world_->currentStep());
  variables_.health->setValue(robot_->health());

  const auto& pos = robot_->position();
  variables_.pos->setValue(QString::asprintf("{ %.2f, %.2f }", pos.x, pos.y));

  double angle = std::remainder(robot_->angle(), 2 * math::kPi);
  variables_.angle->setValue(int(math::radiansToDegrees(angle)));

  if (timer_.isActive()) {
    double fps = 1000.0 / timer_.interval();
    variables_.state->setValue(QString::asprintf("Running (%.1f fps)", fps));
  } else {
    variables_.state->setValue("Paused");
  }

  const auto& stats = robot_->stats();
  variables_.last_move_dist->setValue(stats.last_move_dist);
  variables_.total_move_dist->setValue(stats.total_move_dist);
  variables_.good_fruits->setValue(stats.good_fruits);
  variables_.junk_fruits->setValue(stats.junk_fruits);
  variables_.bad_fruits->setValue(stats.bad_fruits);
  variables_.visited_cells->setValue(stats.visited_cells);

  update();
}

void SandboxWindow::setupVariables() {
  auto config_section = ui->variables->addSection("Configuration");
  variables_.generation = config_section->addProperty("Generation");
  variables_.initial_health = config_section->addProperty("Initial health");
  variables_.world_width = config_section->addProperty("World width");
  variables_.world_height = config_section->addProperty("World height");

  auto game_state_section = ui->variables->addSection("Game state");
  variables_.state = game_state_section->addProperty("State");
  variables_.sim_step = game_state_section->addProperty("Simulation step");
  variables_.health = game_state_section->addProperty("Health");
  variables_.pos = game_state_section->addProperty("Position");
  variables_.angle = game_state_section->addProperty("Angle");
  variables_.last_move_dist = game_state_section->addProperty("Last move dist");
  variables_.total_move_dist = game_state_section->addProperty("Total move dist");
  variables_.good_fruits = game_state_section->addProperty("Good fruits");
  variables_.junk_fruits = game_state_section->addProperty("Junk fruits");
  variables_.bad_fruits = game_state_section->addProperty("Bad fruits");
  variables_.visited_cells = game_state_section->addProperty("Visited cells");
}

}  // namespace harvester_ui
