// Copyright The Darwin Neuroevolution Framework Authors.
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

#include "touch_widget.h"

#include <QLineF>
#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QRectF>

namespace physics_ui {

TouchSensorWidget::TouchSensorWidget(QWidget* parent) : Canvas(parent) {
  setViewport(QRectF(-kCanvasWidth / 2, kCanvasHeight / 2, kCanvasWidth, -kCanvasHeight));
}

void TouchSensorWidget::setSensor(const physics::TouchSensor* sensor) {
  sensor_ = sensor;
  update();
}

void TouchSensorWidget::paintEvent(QPaintEvent* event) {
  Canvas::paintEvent(event);
  if (sensor_ != nullptr) {
    QPainter painter(this);

    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing |
                           QPainter::SmoothPixmapTransform |
                           QPainter::HighQualityAntialiasing);

    // background (whole widget)
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);
    painter.drawRect(rect());

    painter.setTransform(transformFromViewport());

    const QRectF sensor_rect(
        -kSensorWidth / 2, -kSensorHeight / 2, kSensorWidth, kSensorHeight);

    // axes
    painter.setPen(QPen(Qt::lightGray, 0));
    painter.setBrush(Qt::NoBrush);
    painter.drawLine(QLineF(0, -kCanvasHeight / 2, 0, kCanvasHeight / 2));
    painter.drawLine(QLineF(-kCanvasWidth / 2, 0, kCanvasWidth / 2, 0));

    // receptor values
    painter.setPen(QPen(Qt::gray, 0));
    const auto receptors = sensor_->receptors();
    if (receptors.size() == 1) {
      painter.setBrush(receptors[0] > 0 ? Qt::green : Qt::white);
      painter.drawEllipse(sensor_rect);
    } else {
      const double slice_angle = 360.0 / receptors.size();
      for (size_t i = 0; i < receptors.size(); ++i) {
        const double angle = i * slice_angle - 90;
        Q_ASSERT(receptors[i] >= 0);
        painter.setBrush(receptors[i] > 0 ? Qt::green : Qt::white);
        painter.drawPie(sensor_rect, int(angle * 16), int(slice_angle * 16));
      }
    }

    painter.setPen(QPen(Qt::gray, 0));
    painter.setBrush(Qt::white);
    painter.drawEllipse(
        sensor_rect.adjusted(+kSkinSize, +kSkinSize, -kSkinSize, -kSkinSize));
  }
}

}  // namespace physics_ui
