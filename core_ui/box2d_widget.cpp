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
#include <QMouseEvent>

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

void Box2dWidget::setSceneUi(Box2dSceneUi* scene_ui) {
  if (scene_ui_ != nullptr) {
    disconnect(scene_ui_, &Box2dSceneUi::sigPlayPause, this, &Box2dWidget::sigPlayPause);
  }
  CHECK(scene_ui != nullptr);
  scene_ui_ = scene_ui;
  connect(scene_ui, &Box2dSceneUi::sigPlayPause, this, &Box2dWidget::sigPlayPause);
  update();
}

void Box2dWidget::debugRender(QPainter& painter) const {
  core_ui::Box2dRenderer box2d_renderer(&painter);
  box2d_renderer.SetFlags(b2Draw::e_shapeBit | b2Draw::e_centerOfMassBit);

  world_->SetDebugDraw(&box2d_renderer);
  world_->DrawDebugData();
  world_->SetDebugDraw(nullptr);
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
    // custom rendering (optional)
    if (scene_ui_ != nullptr) {
      scene_ui_->render(painter);
    }
    
    // debug rendering layer
    if (enable_debug_render_) {
      debugRender(painter);
    }
  }
}

void Box2dWidget::mousePressEvent(QMouseEvent* event) {
  if (scene_ui_ != nullptr) {
    auto pos = transformToViewport().map(event->localPos());
    scene_ui_->mousePressEvent(pos, event);
  } else {
    emit sigPlayPause();
  }
  update();
}

void Box2dWidget::mouseReleaseEvent(QMouseEvent* event) {
  if (scene_ui_ != nullptr) {
    auto pos = transformToViewport().map(event->localPos());
    scene_ui_->mouseReleaseEvent(pos, event);
  }
  update();
}

void Box2dWidget::mouseMoveEvent(QMouseEvent* event) {
  if (scene_ui_ != nullptr) {
    auto pos = transformToViewport().map(event->localPos());
    scene_ui_->mouseMoveEvent(pos, event);
  }
  update();
}

void Box2dWidget::keyPressEvent(QKeyEvent* event) {
  if (scene_ui_ != nullptr) {
    scene_ui_->keyPressEvent(event);
  }
  update();
}

void Box2dWidget::keyReleaseEvent(QKeyEvent* event) {
  if (scene_ui_ != nullptr) {
    scene_ui_->keyReleaseEvent(event);
  }
  update();
}

}  // namespace core_ui
