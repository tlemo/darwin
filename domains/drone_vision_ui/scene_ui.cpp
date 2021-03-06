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

#include <core/math_2d.h>

#include <QBrush>
#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QRectF>

namespace drone_vision_ui {

void SceneUi::render(QPainter& painter, const QRectF&, bool) {
  renderDrone(painter, scene_->drone());
}

void SceneUi::renderCamera(QPainter& painter, const sim::Camera* camera) const {
  auto body = camera->body();
  const float far = camera->far();
  const float fov = camera->fov();
  const auto pos = body->GetWorldPoint(camera->position());

  const QPointF center(pos.x, pos.y);
  const QRectF frustum_rect(center - QPointF(far, far), center + QPointF(far, far));
  const double angle = math::radiansToDegrees(body->GetAngle()) + 90 + fov / 2;

  painter.setPen(Qt::NoPen);
  painter.setBrush(QColor(64, 64, 64, 32));
  painter.drawPie(frustum_rect, int(-angle * 16), int(fov * 16));
}

void SceneUi::renderDrone(QPainter& painter, const sim::Drone* drone) const {
  if (auto camera = drone->camera()) {
    renderCamera(painter, camera);
  }

  const auto& drone_config = drone->config();
  const float radius = drone_config.radius;
  const auto drone_body = drone->body();
  const auto pos = drone_body->GetPosition();
  painter.save();
  painter.translate(pos.x, pos.y);
  painter.scale(1, -1);
  painter.rotate(math::radiansToDegrees(-drone_body->GetAngle()));
  const QRectF dest_rect(-radius, -radius, radius * 2, radius * 2);
  painter.drawPixmap(dest_rect, drone_pixmap_, drone_pixmap_.rect());
  painter.restore();

  const auto& color = drone_config.color;
  painter.setPen(QPen(QColor::fromRgbF(color.r, color.g, color.b), 0, Qt::DotLine));
  painter.drawEllipse(QPointF(pos.x, pos.y), radius, radius);
}

}  // namespace drone_vision_ui
