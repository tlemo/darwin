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

#include <algorithm>
using namespace std;

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

  const int default_generation = snapshot.generation - 1;
  const int default_max_steps = 1000;
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
  setupScene(domain_->randomTargetPosition());
}

void SandboxWindow::singleStep() {
  CHECK(world_);
  CHECK(agent_);

  if (world_->simStep() && step_ < max_steps_) {
    ++step_;
  } else {
    stop(State::Completed);
  }

  viewport_rect_ = calculateViewport(viewport_rect_);
  box2dWidget()->setViewport(viewport_rect_);
  updateUI();
}

void SandboxWindow::updateUI() {
  const auto projectile_position = world_->projectilePosition();
  const auto target_position = world_->targetPosition();
  const double dist_from_target = (projectile_position - target_position).Length();
  closest_dist_ = min(closest_dist_, dist_from_target);
  variables_.state->setValue(stateDescription());
  variables_.step->setValue(step_);
  variables_.projectile_x->setValue(QString::asprintf("%.3f", projectile_position.x));
  variables_.projectile_y->setValue(QString::asprintf("%.3f", projectile_position.y));
  variables_.dist_from_target->setValue(QString::asprintf("%.3f", dist_from_target));
  variables_.closest_dist->setValue(QString::asprintf("%.3f", closest_dist_));
  update();
}

void SandboxWindow::setupScene(const b2Vec2& target_position) {
  CHECK(domain_ != nullptr);
  CHECK(max_steps_ > 0);

  if (scene_ui_) {
    disconnect(scene_ui_.get(), &SceneUi::sigNewTarget, this, &SandboxWindow::newTarget);
    setSceneUi(nullptr);
    scene_ui_.reset();
  }

  world_ = make_unique<ballistics::World>(target_position, domain_);
  agent_ = make_unique<ballistics::Agent>(genotype_.get());
  step_ = 0;
  closest_dist_ = target_position.Length();

  const auto& config = domain_->config();
  world_->setVerticalLimit(config.range_min_y);
  world_->fireProjectile(agent_->aim(target_position.x, target_position.y));

  variables_.target_x->setValue(QString::asprintf("%.3f", target_position.x));
  variables_.target_y->setValue(QString::asprintf("%.3f", target_position.y));
  variables_.target_dist->setValue(QString::asprintf("%.3f", target_position.Length()));

  viewport_rect_ = calculateViewport();
  setWorld(world_->box2dWorld(), viewport_rect_);

  scene_ui_ = make_unique<SceneUi>(world_.get());
  setSceneUi(scene_ui_.get());
  connect(scene_ui_.get(), &SceneUi::sigNewTarget, this, &SandboxWindow::newTarget);
}

void SandboxWindow::newTarget(double x, double y) {
  pause();
  setupScene(b2Vec2(float(x), float(y)));
  play();

  updateUI();
}

void SandboxWindow::setupVariables() {
  auto config_section = variablesWidget()->addSection("Configuration");
  variables_.generation = config_section->addProperty("Generation");
  variables_.max_steps = config_section->addProperty("Max steps");
  variables_.target_x = config_section->addProperty("Target x coordinate");
  variables_.target_y = config_section->addProperty("Target y coordinate");
  variables_.target_dist = config_section->addProperty("Distance to target");

  auto game_state_section = variablesWidget()->addSection("Game state");
  variables_.state = game_state_section->addProperty("State");
  variables_.step = game_state_section->addProperty("Simulation step");
  variables_.projectile_x = game_state_section->addProperty("Projectile x coordinate");
  variables_.projectile_y = game_state_section->addProperty("Projectile y coordinate");
  variables_.dist_from_target = game_state_section->addProperty("Distance from target");
  variables_.closest_dist = game_state_section->addProperty("Closest distance");
}

QRectF SandboxWindow::calculateViewport(QRectF old_rect) const {
  CHECK(world_);
  const auto& config = domain_->config();
  const auto pos = world_->projectilePosition();
  const double margin = 2 * max(config.projectile_radius, config.target_radius);
  const double left = min(min(config.range_min_x, pos.x) - margin, old_rect.left());
  const double right = max(max(config.range_max_x, pos.x) + margin, old_rect.right());
  const double top = max(max(config.range_max_y, pos.y) + margin, old_rect.top());
  const double bottom = min(min(config.range_min_y, pos.y) - margin, old_rect.bottom());
  return QRectF(QPointF(left, top), QPointF(right, bottom));
}

}  // namespace ballistics_ui
