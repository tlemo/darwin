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

#include "world_widget.h"
#include "box2d_renderer.h"

#include <core/math_2d.h>
#include <core/utils.h>

#include <QBrush>
#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QKeyEvent>

#include <math.h>

namespace cart_pole_ui {

WorldWidget::WorldWidget(QWidget* parent) : core_ui::Canvas(parent) {
  setAutoFillBackground(false);
  setFocusPolicy(Qt::StrongFocus);
  setBorderSize(15);
}

void WorldWidget::setWorld(cart_pole::World* world) {
  CHECK(world != nullptr);
  world_ = world;

  const auto& config = world_->domain()->config();

  // calculate viewport extents based on the configuration values
  constexpr float kMargin = 0.5f;
  const auto half_width = fmax(config.max_distance + kMargin, config.pole_length);
  const auto height = config.pole_length + kMargin;
  setViewport(QRectF(-half_width, height, 2 * half_width, -height));

  update();
}

void WorldWidget::paintEvent(QPaintEvent*) {
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
    auto box2d_world = world_->box2dWorld();

    Box2dRenderer box2d_renderer(&painter);
    box2d_renderer.SetFlags(b2Draw::e_shapeBit | b2Draw::e_centerOfMassBit);

    box2d_world->SetDebugDraw(&box2d_renderer);
    box2d_world->DrawDebugData();
    box2d_world->SetDebugDraw(nullptr);
  }
}

void WorldWidget::mousePressEvent(QMouseEvent*) {
  emit sigPlayPause();
  update();
}

}  // namespace cart_pole_ui
