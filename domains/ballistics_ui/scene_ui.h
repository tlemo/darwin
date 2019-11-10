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

#include <core_ui/sim/box2d_widget.h>
#include <domains/ballistics/world.h>

#include <QPainterPath>

namespace ballistics_ui {

class SceneUi : public physics_ui::Box2dSceneUi {
  Q_OBJECT

 public:
  SceneUi(ballistics::World* world) : world_(world) { trajectory_path_.moveTo(0, 0); }

 signals:
  void sigNewTarget(double x, double y);

 private:
  void render(QPainter& painter, const QRectF& viewport) override;
  void mousePressEvent(const QPointF& pos, QMouseEvent* event) override;

 private:
  ballistics::World* world_ = nullptr;
  QPainterPath trajectory_path_;
};

}  // namespace ballistics_ui
