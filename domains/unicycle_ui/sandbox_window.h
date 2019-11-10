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

#include <core_ui/sim/box2d_sandbox_window.h>
#include <core_ui/properties_widget.h>
#include <domains/unicycle/agent.h>
#include <domains/unicycle/unicycle.h>
#include <domains/unicycle/world.h>

#include <QFrame>
#include <QIcon>
#include <QTimer>

#include <memory>
using namespace std;

namespace unicycle_ui {

class SandboxWindow : public physics_ui::Box2dSandboxWindow {
  struct Variables {
    // configuration
    core_ui::PropertyItem* generation = nullptr;
    core_ui::PropertyItem* max_steps = nullptr;
    core_ui::PropertyItem* initial_angle = nullptr;
    core_ui::PropertyItem* target_position = nullptr;

    // simulation state
    core_ui::PropertyItem* state = nullptr;
    core_ui::PropertyItem* step = nullptr;
    core_ui::PropertyItem* fitness_bonus = nullptr;
    core_ui::PropertyItem* position = nullptr;
    core_ui::PropertyItem* velocity = nullptr;
    core_ui::PropertyItem* angle = nullptr;
    core_ui::PropertyItem* angular_velocity = nullptr;
    core_ui::PropertyItem* dist_from_target = nullptr;
  };

 public:
  bool setup();

  void newScene() override;
  void singleStep() override;
  void updateUI() override;

 private:
  void setupVariables();

 private:
  Variables variables_;

  const unicycle::Unicycle* domain_ = nullptr;

  shared_ptr<const darwin::Genotype> genotype_;
  unique_ptr<unicycle::World> world_;
  unique_ptr<unicycle::Agent> agent_;
  unique_ptr<SceneUi> scene_ui_;
  int step_ = -1;
  int max_steps_ = -1;
};

}  // namespace unicycle_ui
