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
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QRectF>

#include <array>
using namespace std;

namespace ballistics_ui {

void SceneUi::render(QPainter& painter, const QRectF& viewport, bool /*debug*/) {
  painter.setPen(QPen(Qt::lightGray, 0));

  // x/y axis
  painter.drawLine(QPointF(viewport.left(), 0.0), QPointF(viewport.right(), 0.0));
  painter.drawLine(QPointF(0.0, viewport.bottom()), QPointF(0.0, viewport.top()));

  constexpr double kTickSpacing = 0.2;
  constexpr double kTickSize = -0.05;

  // horizontal ticks
  for (double x = viewport.left(); x < viewport.right(); x += kTickSpacing) {
    painter.drawLine(QPointF(x, 0), QPointF(x, kTickSize));
  }

  // vertical ticks
  for (double y = viewport.bottom(); y < viewport.top(); y += kTickSpacing) {
    painter.drawLine(QPointF(0, y), QPointF(kTickSize, y));
  }

  const auto target_position = world_->targetPosition();
  const auto projectile_position = world_->projectilePosition();

  const auto target_point = QPointF(target_position.x, target_position.y);
  const auto projectile_point = QPointF(projectile_position.x, projectile_position.y);

  // origin - target line
  painter.setPen(QPen(Qt::lightGray, 0, Qt::DotLine));
  painter.drawLine(QPointF(0, 0), target_point);

  // trajectory
  painter.setPen(QPen(Qt::green, 0, Qt::DotLine));
  painter.setBrush(Qt::NoBrush);
  trajectory_path_.lineTo(projectile_point);
  painter.drawPath(trajectory_path_);
}

void SceneUi::mousePressEvent(const QPointF& pos, QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    emit sigNewTarget(pos.x(), pos.y());
  } else {
    emit sigPlayPause();
  }
}

}  // namespace ballistics_ui
