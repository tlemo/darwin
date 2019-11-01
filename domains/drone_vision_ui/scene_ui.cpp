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

void SceneUi::render(QPainter& painter, const QRectF&) {
  renderDrone(painter);
  renderCamera(painter, scene_->camera());
}

void SceneUi::renderCamera(QPainter& painter, const physics::Camera* camera) const {
  auto body = camera->body();
  const float far = camera->far();
  const float fov = camera->fov();
  const auto pos = body->GetWorldPoint(b2Vec2(0, 0));

  const QPointF center(pos.x, pos.y);
  const QRectF frustum_rect(center - QPointF(far, far), center + QPointF(far, far));
  const double angle = math::radiansToDegrees(body->GetAngle()) + 90 + fov / 2;

  painter.setPen(Qt::NoPen);
  painter.setBrush(QColor(64, 64, 64, 32));
  painter.drawPie(frustum_rect, int(-angle * 16), int(fov * 16));
}

void SceneUi::renderDrone(QPainter& painter) const {
  auto vars = scene_->variables();
  auto config = scene_->config();
  const float radius = config->drone_radius;
  painter.save();
  painter.translate(vars->drone_x, vars->drone_y);
  painter.scale(1, -1);
  painter.rotate(math::radiansToDegrees(-vars->drone_dir));
  const QRectF dest_rect(-radius, -radius, radius * 2, radius * 2);
  painter.drawPixmap(dest_rect, drone_pixmap_, drone_pixmap_.rect());
  painter.restore();
}

}  // namespace drone_vision_ui
