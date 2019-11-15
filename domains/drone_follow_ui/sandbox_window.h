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

#include <core/sim/drone_controller.h>
#include <core_ui/sim/box2d_sandbox_window.h>
#include <core_ui/sim/camera_widget.h>
#include <core_ui/properties_widget.h>
#include <domains/drone_follow/domain.h>
#include <domains/drone_follow/scene.h>

#include <QFrame>
#include <QIcon>
#include <QTimer>

#include <memory>
#include <unordered_map>
#include <string>
using namespace std;

namespace drone_follow_ui {

class SandboxWindow : public physics_ui::Box2dSandboxWindow {
  struct Variables {
    // configuration
    core_ui::PropertyItem* generation = nullptr;
    core_ui::PropertyItem* max_steps = nullptr;

    // simulation state
    core_ui::PropertyItem* state = nullptr;
    core_ui::PropertyItem* step = nullptr;
    core_ui::PropertyItem* fitness = nullptr;
  };

 public:
  SandboxWindow() { box2dWidget()->setDebugRender(false); }

  bool setup();

  void newScene() override;
  void singleStep() override;
  void updateUI() override;

 private:
  void setupVariables();
  void setupSceneVariables();
  void updateVariables();

 private:
  Variables variables_;
  unordered_map<string, core_ui::PropertyItem*> scene_variables_map_;

  const drone_follow::DroneFollow* domain_ = nullptr;

  physics_ui::CameraWidget* camera_widget_ = nullptr;

  shared_ptr<const darwin::Genotype> genotype_;
  unique_ptr<drone_follow::Scene> scene_;
  unique_ptr<sim::DroneController> agent_;
  unique_ptr<SceneUi> scene_ui_;
  int step_ = -1;
  int max_steps_ = -1;
};

}  // namespace drone_follow_ui
