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

#include "scene_ui.h"

#include <QBrush>
#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QRectF>
#include <QMouseEvent>

#include <array>
using namespace std;

namespace ballistics_ui {

void SceneUi::render(QPainter& painter) {
  // draw an "arrow" pointing to the target position
  constexpr float kArrowHalfSize = ballistics::World::kGroundY / 4;
  const float target_position = world_->targetPosition();
  array<QPointF, 3> points = {
    QPointF(target_position, 3 * kArrowHalfSize),
    QPointF(target_position + kArrowHalfSize, kArrowHalfSize),
    QPointF(target_position - kArrowHalfSize, kArrowHalfSize),
  };
  painter.setPen(QPen(Qt::gray, 0));
  painter.setBrush(Qt::green);
  painter.drawConvexPolygon(points.data(), int(points.size()));
}

void SceneUi::mousePressEvent(const QPointF& pos, QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    world_->setTargetPosition(pos.x());
  } else {
    emit sigPlayPause();
  }
}

}  // namespace ballistics_ui
