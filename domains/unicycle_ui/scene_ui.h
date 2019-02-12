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

#include <core_ui/box2d_widget.h>
#include <domains/unicycle/world.h>

namespace unicycle_ui {

class SceneUi : public core_ui::Box2dSceneUi {
 public:
  SceneUi(unicycle::World* world) : world_(world) {}

 private:
  void render(QPainter& painter) override;
  void mousePressEvent(const QPointF& pos, QMouseEvent* event) override;

 private:
  unicycle::World* world_ = nullptr;
};

}  // namespace unicycle_ui
