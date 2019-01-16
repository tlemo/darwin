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

#include "box2d_widget.h"

#include <core/math_2d.h>
#include <core/utils.h>
#include <core_ui/box2d_renderer.h>

#include <QBrush>
#include <QPainter>
#include <QPen>
#include <QPointF>

#include <math.h>

namespace core_ui {

Box2dWidget::Box2dWidget(QWidget* parent) : core_ui::Canvas(parent) {
  setAutoFillBackground(false);
  setFocusPolicy(Qt::StrongFocus);
  setBorderSize(15);
}

void Box2dWidget::setWorld(b2World* world, const QRectF& viewport) {
  CHECK(world != nullptr);
  world_ = world;
  setViewport(viewport);
  update();
}

void Box2dWidget::paintEvent(QPaintEvent*) {
  QPainter painter(this);

  painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

  // background (whole widget)
  painter.setPen(Qt::NoPen);
  painter.setBrush(kBackgroundColor);
  painter.drawRect(rect());

  painter.setTransform(transformFromViewport());

  // viewport background
  painter.setPen(Qt::NoPen);
  painter.setBrush(kViewportColor);
  painter.drawRect(viewport());

  if (world_ != nullptr) {
    core_ui::Box2dRenderer box2d_renderer(&painter);
    box2d_renderer.SetFlags(b2Draw::e_shapeBit | b2Draw::e_centerOfMassBit);

    world_->SetDebugDraw(&box2d_renderer);
    world_->DrawDebugData();
    world_->SetDebugDraw(nullptr);
  }
}

void Box2dWidget::mousePressEvent(QMouseEvent*) {
  emit sigPlayPause();
  update();
}

}  // namespace core_ui
