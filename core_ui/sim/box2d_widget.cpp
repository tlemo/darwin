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
#include <core_ui/sim/box2d_renderer.h>

#include <QBrush>
#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QMouseEvent>
#include <QPolygonF>

#include <math.h>

namespace physics_ui {

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
  scene_ui_ = scene_ui;
  if (scene_ui_ != nullptr) {
    connect(scene_ui_, &Box2dSceneUi::sigPlayPause, this, &Box2dWidget::sigPlayPause);
  }
  update();
}

void Box2dWidget::setDebugRender(bool enable) {
  enable_debug_render_ = enable;
  update();
}

void Box2dWidget::renderDebugLayer(QPainter& painter) const {
  physics_ui::Box2dRenderer box2d_renderer(&painter);
  box2d_renderer.SetFlags(b2Draw::e_shapeBit | b2Draw::e_centerOfMassBit);

  world_->SetDebugDraw(&box2d_renderer);
  world_->DrawDebugData();
  world_->SetDebugDraw(nullptr);
}

static QPointF vecToPoint(const b2Vec2& v) {
  return QPointF(v.x, v.y);
}

static void renderBody(QPainter& painter, const b2Body* body) {
  for (const b2Fixture* fixture = body->GetFixtureList(); fixture != nullptr;
       fixture = fixture->GetNext()) {
    // setup pen and brush
    const auto& color = fixture->GetMaterial().color;
    painter.setPen(QPen(QColor::fromRgbF(color.r, color.g, color.b), 0));
    painter.setBrush(QColor::fromRgbF(color.r, color.g, color.b, 0.4));

    // draw the fixture's shape
    switch (fixture->GetType()) {
      case b2Shape::e_circle: {
        auto shape = static_cast<const b2CircleShape*>(fixture->GetShape());
        auto center = vecToPoint(body->GetWorldPoint(shape->m_p));
        painter.drawEllipse(center, shape->m_radius, shape->m_radius);
        break;
      }
      case b2Shape::e_edge: {
        auto shape = static_cast<const b2EdgeShape*>(fixture->GetShape());
        auto p1 = vecToPoint(body->GetWorldPoint(shape->m_vertex1));
        auto p2 = vecToPoint(body->GetWorldPoint(shape->m_vertex2));
        painter.drawLine(p1, p2);
        break;
      }
      case b2Shape::e_polygon: {
        auto shape = static_cast<const b2PolygonShape*>(fixture->GetShape());
        QPolygonF polygon;
        for (int i = 0; i < shape->m_count; ++i) {
          auto point = vecToPoint(body->GetWorldPoint(shape->m_vertices[i]));
          polygon.append(point);
        }
        painter.drawPolygon(polygon);
        break;
      }
      default:
        FATAL("Unexpected fixture shape");
    }
  }
}

void Box2dWidget::renderGeneric(QPainter& painter) const {
  for (const b2Body* body = world_->GetBodyList(); body != nullptr;
       body = body->GetNext()) {
    if (body->UseDefaultRendering()) {
      renderBody(painter, body);
    }
  }
}

void Box2dWidget::paintEvent(QPaintEvent*) {
  QPainter painter(this);

  painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing |
                         QPainter::SmoothPixmapTransform |
                         QPainter::HighQualityAntialiasing);

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
      scene_ui_->render(painter, viewport());
    }

    if (enable_debug_render_) {
      renderDebugLayer(painter);
    } else {
      renderGeneric(painter);
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

void Box2dWidget::focusInEvent(QFocusEvent*) {
  if (scene_ui_ != nullptr) {
    scene_ui_->focusInEvent();
  }
  update();
}

void Box2dWidget::focusOutEvent(QFocusEvent*) {
  if (scene_ui_ != nullptr) {
    scene_ui_->focusOutEvent();
  }
  update();
}

}  // namespace physics_ui
