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
#include <QRectF>
#include <QMouseEvent>
#include <QPolygonF>
#include <QPalette>

#include <math.h>
#include <limits>

namespace physics_ui {

Box2dWidget::Box2dWidget(QWidget* parent) : core_ui::Canvas(parent) {
  QPalette background_palette;
  background_palette.setColor(QPalette::Window, kBackgroundColor);
  setPalette(background_palette);
  setAutoFillBackground(true);
  setFocusPolicy(Qt::StrongFocus);
  setBorderSize(15);
}

void Box2dWidget::setWorld(b2World* world) {
  CHECK(world != nullptr);
  world_ = world;
  update();
}

void Box2dWidget::setSceneUi(Box2dSceneUi* scene_ui) {
  if (scene_ui_ != nullptr) {
    disconnect(scene_ui_, &Box2dSceneUi::sigPlayPause, this, &Box2dWidget::sigPlayPause);
  }
  scene_ui_ = scene_ui;
  if (scene_ui_ != nullptr) {
    auto help_text = scene_ui_->help();
    if (!help_text.isEmpty()) {
      setToolTip(help_text);
    }
    connect(scene_ui_, &Box2dSceneUi::sigPlayPause, this, &Box2dWidget::sigPlayPause);
  }
  update();
}

void Box2dWidget::setViewportPolicy(Box2dWidget::ViewportPolicy policy) {
  viewport_policy_ = policy;
  update();
}

void Box2dWidget::setDebugRender(bool enable) {
  enable_debug_render_ = enable;
  update();
}

void Box2dWidget::setRenderLights(bool enable) {
  render_lights_ = enable;
  update();
}

void Box2dWidget::renderDebugLayer(QPainter& painter) const {
  physics_ui::Box2dRenderer box2d_renderer(&painter);
  box2d_renderer.SetFlags(b2Draw::e_shapeBit | b2Draw::e_centerOfMassBit |
                          b2Draw::e_jointBit);

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

static void renderLight(QPainter& painter, const b2Light* light) {
  auto light_def = light->GetDef();
  auto center = vecToPoint(light_def.body->GetWorldPoint(light_def.position));

  double radius = light_def.attenuation_distance;
  for (int i = 0; i < 5; ++i) {
    const int alpha = (i + 1) * 50 + 5;
    painter.setPen(QPen(QColor(160, 160, 0, alpha), 0, Qt::DashLine));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(center, radius, radius);
    radius /= 2;
  }

  painter.setPen(Qt::NoPen);
  painter.setBrush(QColor(128, 128, 128, 64));
  painter.drawEllipse(center, 0.4, 0.4);

  const auto& color = light_def.color;
  painter.setBrush(QColor::fromRgbF(color.r, color.g, color.b));
  painter.drawEllipse(center, 0.2, 0.2);
}

void Box2dWidget::renderGeneric(QPainter& painter) const {
  // render bodies
  for (const b2Body* body = world_->GetBodyList(); body != nullptr;
       body = body->GetNext()) {
    if (body->UseDefaultRendering()) {
      renderBody(painter, body);
    }
  }

  // render lights
  if (render_lights_) {
    for (const b2Light* light = world_->GetLightList(); light != nullptr;
         light = light->GetNext()) {
      renderLight(painter, light);
    }
  }
}

void Box2dWidget::applyViewportPolicy() {
  // we're using Box2d's debug draw interface to compute the world extents
  struct Box2dWorldExtents : public b2Draw {
    float min_x = std::numeric_limits<float>::infinity();
    float max_x = -std::numeric_limits<float>::infinity();
    float min_y = std::numeric_limits<float>::infinity();
    float max_y = -std::numeric_limits<float>::infinity();
    bool valid = false;

    void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color&) override {
      for (int i = 0; i < vertexCount; ++i) {
        const auto x = vertices[i].x;
        const auto y = vertices[i].y;

        if (x < min_x) {
          min_x = x;
        } else if (x > max_x) {
          max_x = x;
        }

        if (y < min_y) {
          min_y = y;
        } else if (y > max_y) {
          max_y = y;
        }
      }
      valid = true;
    }

    void DrawSolidPolygon(const b2Vec2*, int32, const b2Color&) override {
      FATAL("Unexpected");
    }

    void DrawCircle(const b2Vec2&, float32, const b2Color&) override {
      FATAL("Unexpected");
    }

    void DrawSolidCircle(const b2Vec2&, float32, const b2Vec2&, const b2Color&) override {
      FATAL("Unexpected");
    }

    void DrawSegment(const b2Vec2&, const b2Vec2&, const b2Color&) override {
      FATAL("Unexpected");
    }

    void DrawTransform(const b2Transform&) override { FATAL("Unexpected"); }

    void DrawPoint(const b2Vec2&, float32, const b2Color&) override {
      FATAL("Unexpected");
    }
  };

  Box2dWorldExtents extents_tracker;
  extents_tracker.SetFlags(b2Draw::e_aabbBit);

  switch (viewport_policy_) {
    case ViewportPolicy::UserDefined:
      // nothing to do here
      return;

    case ViewportPolicy::AutoExpanding:
      // initialize Box2dWorldExtents to the current viewport
      extents_tracker.min_x = viewport().left();
      extents_tracker.max_x = viewport().right();
      extents_tracker.min_y = viewport().bottom();
      extents_tracker.min_x = viewport().top();
      break;

    case ViewportPolicy::AutoFit:
      // default Box2dWorldExtents is what we need
      break;

    default:
      FATAL("Unexpected viewport policy");
  }

  world_->SetDebugDraw(&extents_tracker);
  world_->DrawDebugData();
  world_->SetDebugDraw(nullptr);

  if (extents_tracker.valid) {
    const auto left = extents_tracker.min_x;
    const auto top = extents_tracker.max_y;
    const auto width = extents_tracker.max_x - extents_tracker.min_x;
    const auto height = extents_tracker.min_y - extents_tracker.max_y;

    const QRectF new_viewport(left, top, width, height);

    if (new_viewport != viewport()) {
      setViewport(new_viewport, false);
    }
  }
}

void Box2dWidget::paintEvent(QPaintEvent* event) {
  applyViewportPolicy();

  // chain call the base implementation
  // (this will render the background and an optional frame)
  Canvas::paintEvent(event);

  QPainter painter(this);

  painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing |
                         QPainter::SmoothPixmapTransform);

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
