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
#include <QColor>
#include <QPointF>
#include <QLineF>
#include <QRectF>

namespace drone_track_ui {

static QPointF vecToPoint(const b2Vec2& v) {
  return QPointF(v.x, v.y);
}

static QPointF dronePosition(const sim::Drone* drone) {
  return vecToPoint(drone->body()->GetPosition());
}

static double dist(const QPointF& a, const QPointF& b) {
  return QLineF(a, b).length();
}

SceneUi::SceneUi(drone_track::Scene* scene) : scene_(scene) {
  drone_path_.moveTo(dronePosition(scene_->drone()));
}

void SceneUi::render(QPainter& painter, const QRectF& viewport) {
  painter.setPen(Qt::NoPen);
  painter.setBrush(Qt::white);
  painter.drawRect(viewport);

  renderTrack(painter);
  renderPath(painter);
  renderCurrentSegment(painter);
  renderDrone(painter, scene_->drone());
}

void SceneUi::step() {
  // update drone path
  constexpr double kMinDist = 0.1;
  const auto drone_pos = dronePosition(scene_->drone());
  if (dist(drone_pos, drone_path_.currentPosition()) > kMinDist) {
    drone_path_.lineTo(drone_pos);
  }
}

void SceneUi::renderCamera(QPainter& painter, const sim::Camera* camera) const {
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
  painter.drawEllipse(vecToPoint(pos), radius, radius);
}

void SceneUi::renderPath(QPainter& painter) const {
  painter.setBrush(Qt::NoBrush);
  painter.setPen(QPen(Qt::darkGray, 0, Qt::DotLine));
  painter.drawPath(drone_path_);
}

void SceneUi::renderTrack(QPainter& painter) const {
  QPainterPath track_path;
  const auto& track_nodes = scene_->track()->trackNodes();

  // inner track edge
  for (size_t i = 0; i < track_nodes.size(); ++i) {
    const auto& node = track_nodes[i];
    if (i == 0) {
      track_path.moveTo(node.pos.x, node.pos.y);
    } else {
      track_path.lineTo(node.pos.x, node.pos.y);
    }
  }

  // outer track edge
  for (size_t i = 0; i < track_nodes.size(); ++i) {
    const auto& node = track_nodes[i];
    const auto pos = node.offsetPos(scene_->config()->track_width);
    if (i == 0) {
      track_path.moveTo(pos.x, pos.y);
    } else {
      track_path.lineTo(pos.x, pos.y);
    }
  }

  painter.setPen(Qt::NoPen);
  painter.setBrush(QColor(240, 240, 240));
  painter.drawPath(track_path);
}

void SceneUi::renderCurrentSegment(QPainter& painter) const {
  const auto track = scene_->track();
  constexpr float kOffset = 0.4f;
  const auto vars = scene_->variables();
  const auto index = track->distanceToNode(vars->distance);
  const auto& node = track->trackNodes()[index];
  const auto p1 = node.offsetPos(-kOffset);
  const auto p2 = node.offsetPos(scene_->config()->track_width + kOffset);
  painter.setBrush(Qt::NoBrush);
  painter.setPen(QPen(Qt::green, 0));
  painter.drawLine(QPointF(p1.x, p1.y), QPointF(p2.x, p2.y));
}

}  // namespace drone_track_ui
