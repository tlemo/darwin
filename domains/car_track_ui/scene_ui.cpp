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

namespace car_track_ui {

static QPointF vecToPoint(const b2Vec2& v) {
  return QPointF(v.x, v.y);
}

static QPointF carPosition(const sim::Car* car) {
  return vecToPoint(car->body()->GetPosition());
}

static double dist(const QPointF& a, const QPointF& b) {
  return QLineF(a, b).length();
}

SceneUi::SceneUi(car_track::Scene* scene) : scene_(scene) {
  car_path_.moveTo(carPosition(scene_->car()));
}

void SceneUi::render(QPainter& painter, const QRectF& viewport) {
  painter.setPen(Qt::NoPen);
  painter.setBrush(Qt::white);
  painter.drawRect(viewport);

  renderTrack(painter);
  renderPath(painter);
  renderCurrentSegment(painter);

  if (auto camera = scene_->car()->camera()) {
    renderCamera(painter, camera);
  }
}

void SceneUi::step() {
  // update car path
  constexpr double kMinDist = 0.1;
  const auto car_pos = carPosition(scene_->car());
  if (dist(car_pos, car_path_.currentPosition()) > kMinDist) {
    car_path_.lineTo(car_pos);
  }
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

void SceneUi::renderPath(QPainter& painter) const {
  painter.setBrush(Qt::NoBrush);
  painter.setPen(QPen(Qt::darkGray, 0, Qt::DotLine));
  painter.drawPath(car_path_);
}

void SceneUi::renderTrack(QPainter& painter) const {
  QPainterPath track_path;

  // inner track edge
  const auto& inner_nodes = scene_->track()->innerOutline().nodes();
  for (size_t i = 0; i < inner_nodes.size(); ++i) {
    const auto& node = inner_nodes[i];
    if (i == 0) {
      track_path.moveTo(node.p.x, node.p.y);
    } else {
      track_path.lineTo(node.p.x, node.p.y);
    }
  }

  // outer track edge
  const auto& outer_nodes = scene_->track()->outerOutline().nodes();
  for (size_t i = 0; i < outer_nodes.size(); ++i) {
    const auto& node = outer_nodes[i];
    if (i == 0) {
      track_path.moveTo(node.p.x, node.p.y);
    } else {
      track_path.lineTo(node.p.x, node.p.y);
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
  const auto& node = track->distanceToNode(vars->distance);
  const auto p1 = node.offset(kOffset);
  const auto p2 = node.offset(-scene_->config()->track_width - kOffset);
  painter.setBrush(Qt::NoBrush);
  painter.setPen(QPen(Qt::green, 0));
  painter.drawLine(QPointF(p1.x, p1.y), QPointF(p2.x, p2.y));
}

}  // namespace car_track_ui
