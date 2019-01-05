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

#include "box2d_renderer.h"

#include <QColor>
#include <QBrush>
#include <QPen>
#include <QPolygonF>

namespace cart_pole_ui {

void Box2dRenderer::DrawPolygon(const b2Vec2* vertices,
                                int32 vertexCount,
                                const b2Color& color) {
  QColor pen_color;
  pen_color.setRgbF(color.r, color.g, color.b, color.a);

  QPolygonF polygon;
  for (int i = 0; i < vertexCount; ++i) {
    polygon.append(QPointF(vertices[i].x, vertices[i].y));
  }

  painter_->setPen(QPen(pen_color, 0));
  painter_->setBrush(Qt::NoBrush);
  painter_->drawPolygon(polygon);
}

void Box2dRenderer::DrawSolidPolygon(const b2Vec2* vertices,
                                     int32 vertexCount,
                                     const b2Color& color) {
  constexpr float kAlphaScale = 0.7f;
  
  QColor brush_color;
  brush_color.setRgbF(color.r, color.g, color.b, color.a * kAlphaScale);

  QPolygonF polygon;
  for (int i = 0; i < vertexCount; ++i) {
    polygon.append(QPointF(vertices[i].x, vertices[i].y));
  }

  painter_->setPen(QPen(Qt::darkGray, 0));
  painter_->setBrush(QBrush(brush_color));
  painter_->drawPolygon(polygon);
}

void Box2dRenderer::DrawCircle(const b2Vec2& center,
                               float32 radius,
                               const b2Color& color) {
  QColor pen_color;
  pen_color.setRgbF(color.r, color.g, color.b, color.a);

  painter_->setPen(QPen(pen_color, 0));
  painter_->setBrush(Qt::NoBrush);
  painter_->drawEllipse(QPointF(center.x, center.y), radius, radius);
}

void Box2dRenderer::DrawSolidCircle(const b2Vec2& center,
                                    float32 radius,
                                    const b2Vec2& axis,
                                    const b2Color& color) {
  constexpr float kAlphaScale = 0.7f;
  
  QColor brush_color;
  brush_color.setRgbF(color.r, color.g, color.b, color.a * kAlphaScale);

  painter_->setPen(QPen(Qt::blue, 0));
  painter_->setBrush(QBrush(brush_color));
  painter_->drawEllipse(QPointF(center.x, center.y), radius, radius);

  painter_->setBrush(Qt::NoBrush);
  painter_->drawLine(QPointF(center.x, center.y),
                     QPointF(center.x + axis.x * radius, center.y + axis.y * radius));
}

void Box2dRenderer::DrawSegment(const b2Vec2& p1,
                                const b2Vec2& p2,
                                const b2Color& color) {
  QColor pen_color;
  pen_color.setRgbF(color.r, color.g, color.b, color.a);
  
  painter_->setPen(QPen(pen_color, 0));
  painter_->setBrush(Qt::NoBrush);
  painter_->drawLine(QLineF(p1.x, p1.y, p2.x, p2.y));
}

void Box2dRenderer::DrawTransform(const b2Transform& xf) {
  painter_->setBrush(Qt::NoBrush);

  constexpr float kAxisScale = 0.05f;
  constexpr float kCenterSize = 0.01f;
  constexpr int kColorAlpha = 200;

  auto x_axis = xf.q.GetXAxis();
  x_axis *= kAxisScale;
  painter_->setPen(QPen(QColor(255, 0, 0, kColorAlpha), 0));
  painter_->drawLine(QPointF(xf.p.x, xf.p.y),
                     QPointF(xf.p.x + x_axis.x, xf.p.y + x_axis.y));

  auto y_axis = xf.q.GetYAxis();
  y_axis *= kAxisScale;
  painter_->setPen(QPen(QColor(0, 255, 0, kColorAlpha), 0));
  painter_->drawLine(QPointF(xf.p.x, xf.p.y),
                     QPointF(xf.p.x + y_axis.x, xf.p.y + y_axis.y));

  painter_->setPen(QPen(QColor(0, 0, 255, kColorAlpha), 0));
  painter_->drawEllipse(QPointF(xf.p.x, xf.p.y), kCenterSize, kCenterSize);
}

void Box2dRenderer::DrawPoint(const b2Vec2& p, float32 size, const b2Color& color) {
  QColor pen_color;
  pen_color.setRgbF(color.r, color.g, color.b, color.a);
  
  painter_->setPen(QPen(pen_color, 0));
  painter_->setBrush(pen_color);
  painter_->drawEllipse(QPointF(p.x, p.y), size, size);
}

}  // namespace cart_pole_ui
