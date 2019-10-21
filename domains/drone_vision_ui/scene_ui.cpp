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

namespace drone_vision_ui {

void SceneUi::render(QPainter& painter, const QRectF&) {
#if 0 // TODO
  // draw an "arrow" pointing to the target position
  constexpr float kArrowHalfSize = drone_vision::World::kGroundY / 4;
  const float target_position = scene_->targetPosition();
  array<QPointF, 3> points = {
    QPointF(target_position, 3 * kArrowHalfSize),
    QPointF(target_position + kArrowHalfSize, kArrowHalfSize),
    QPointF(target_position - kArrowHalfSize, kArrowHalfSize),
  };
  painter.setPen(QPen(Qt::gray, 0));
  painter.setBrush(Qt::green);
  painter.drawConvexPolygon(points.data(), int(points.size()));
#endif  
}

void SceneUi::mousePressEvent(const QPointF& pos, QMouseEvent* event) {
#if 0 // TODO
  if (event->button() == Qt::LeftButton) {
    scene_->setTargetPosition(pos.x());
  } else {
    emit sigPlayPause();
  }
#endif  
}

}  // namespace drone_vision_ui
