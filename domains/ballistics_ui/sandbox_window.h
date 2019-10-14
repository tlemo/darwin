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

#pragma once

#include "scene_ui.h"

#include <core_ui/physics/box2d_sandbox_window.h>
#include <core_ui/properties_widget.h>
#include <domains/ballistics/agent.h>
#include <domains/ballistics/ballistics.h>
#include <domains/ballistics/world.h>

#include <QFrame>
#include <QIcon>
#include <QTimer>

#include <memory>
using namespace std;

namespace ballistics_ui {

class SandboxWindow : public physics_ui::Box2dSandboxWindow {
  struct Variables {
    // configuration
    core_ui::PropertyItem* generation = nullptr;
    core_ui::PropertyItem* max_steps = nullptr;
    core_ui::PropertyItem* target_x = nullptr;
    core_ui::PropertyItem* target_y = nullptr;
    core_ui::PropertyItem* target_dist = nullptr;

    // simulation state
    core_ui::PropertyItem* state = nullptr;
    core_ui::PropertyItem* step = nullptr;
    core_ui::PropertyItem* projectile_x = nullptr;
    core_ui::PropertyItem* projectile_y = nullptr;
    core_ui::PropertyItem* dist_from_target = nullptr;
    core_ui::PropertyItem* closest_dist = nullptr;
  };

 public:
  bool setup();

  void newScene() override;
  void singleStep() override;
  void updateUI() override;

 private:
  void newTarget(double x, double y);
  void setupScene(const b2Vec2& target_position);
  void setupVariables();
  QRectF calculateViewport(QRectF old_rect = QRect(0, 0, 0, 0)) const;

 private:
  Variables variables_;

  const ballistics::Ballistics* domain_ = nullptr;

  shared_ptr<const darwin::Genotype> genotype_;
  unique_ptr<ballistics::World> world_;
  unique_ptr<ballistics::Agent> agent_;
  unique_ptr<SceneUi> scene_ui_;
  
  int step_ = -1;
  int max_steps_ = -1;
  double closest_dist_ = 0;
  
  QRectF viewport_rect_;
};

}  // namespace ballistics_ui
