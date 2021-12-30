
#include "map_layers.h"

#include <QPen>
#include <QBrush>
#include <QPainter>
#include <QPolygonF>
#include <QRectF>
#include <QMarginsF>
#include <QStyleOptionGraphicsItem>

static QPointF vecToPoint(const b2Vec2& v) {
  return QPointF(v.x, v.y);
}

void WorldMap::paint(QPainter* painter,
                     const QStyleOptionGraphicsItem* option,
                     QWidget*) {
  const auto lod = option->levelOfDetailFromTransform(painter->worldTransform());

  painter->setBrush(QBrush(Qt::white));
  painter->setPen(Qt::NoPen);
  painter->drawRect(bounding_rect_);

  for (const auto& obj : *world_) {
    const auto ds = obj.radius;
    const auto clip_rect = option->exposedRect.adjusted(-ds, -ds, ds, ds);

    const auto center = vecToPoint(obj.worldPoint(b2Vec2(0, 0)));
    if (!clip_rect.contains(center)) {
      continue;
    }

    // edges
    for (const auto& edge : obj.edges) {
      const auto& color = edge.color;
      painter->setPen(QPen(QColor::fromRgbF(color.r, color.g, color.b), 0));
      const auto a = vecToPoint(obj.worldPoint(edge.a));
      const auto b = vecToPoint(obj.worldPoint(edge.b));
      painter->drawLine(a, b);
    }

    // circles
    for (const auto& circle : obj.circles) {
      const auto& color = circle.color;
      painter->setPen(QPen(QColor::fromRgbF(color.r, color.g, color.b), 0));
      painter->setBrush(QColor::fromRgbF(color.r, color.g, color.b, 0.4));
      const auto p = vecToPoint(obj.worldPoint(circle.center));
      painter->drawEllipse(p, circle.radius, circle.radius);
    }

    // polygons
    for (const auto& polygon : obj.polygons) {
      const auto& color = polygon.color;
      painter->setPen(QPen(QColor::fromRgbF(color.r, color.g, color.b), 0));
      painter->setBrush(QColor::fromRgbF(color.r, color.g, color.b, 0.4));
      QPolygonF shape;
      for (const auto& p : polygon.points) {
        shape.append(vecToPoint(obj.worldPoint(p)));
      }
      painter->drawPolygon(shape);
    }
  }
}
