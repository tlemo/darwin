
#include "map_layers.h"

#include <QPen>
#include <QBrush>
#include <QPainter>
#include <QPolygonF>
#include <QRectF>
#include <QMarginsF>
#include <QStyleOptionGraphicsItem>

#include <limits>

static QPointF vecToPoint(const b2Vec2& v) {
  return QPointF(v.x, v.y);
}

void WorldMap::setCursorPosition(const QPointF& pos, bool enable_highlighting) {
  cursor_pos_ = pos;
  enable_highlighting_ = enable_highlighting;
}

void WorldMap::paint(QPainter* painter,
                     const QStyleOptionGraphicsItem* option,
                     QWidget*) {
  const auto lod = option->levelOfDetailFromTransform(painter->worldTransform());

  painter->setBrush(QBrush(Qt::white));
  painter->setPen(Qt::NoPen);
  painter->drawRect(bounding_rect_);

  // track the closest object to the cursor position
  double closest_dist_squared = std::numeric_limits<float>::infinity();
  QPointF selection_pos;
  float selection_radius = 0;

  for (const auto& obj : *world_) {
    const auto ds = obj.radius;
    const auto clip_rect = option->exposedRect.adjusted(-ds, -ds, ds, ds);

    const auto lod_size = ds * lod;

    const auto center = vecToPoint(obj.worldPoint(b2Vec2(0, 0)));
    if (!clip_rect.contains(center)) {
      continue;
    }

    const auto dx = center.x() - cursor_pos_.x();
    const auto dy = center.y() - cursor_pos_.y();
    const auto dist_squared = dx * dx + dy * dy;
    if (dist_squared < closest_dist_squared) {
      closest_dist_squared = dist_squared;
      selection_pos = center;
      selection_radius = obj.radius;
    }

    if (lod_size < 1.1) {
      const auto& color = obj.base_color;
      painter->setPen(QPen(QColor::fromRgbF(color.r, color.g, color.b), 0));
      painter->drawPoint(center);
      continue;
    }

    // edges
    for (const auto& edge : obj.edges) {
      const auto& color = edge.color;
      painter->setPen(QPen(QColor::fromRgbF(color.r, color.g, color.b), 0));
      const auto a = vecToPoint(obj.worldPoint(edge.a));
      const auto b = vecToPoint(obj.worldPoint(edge.b));
      painter->drawLine(a, b);
      if (lod_size < 5.0) {
        break;
      }
    }

    // circles
    for (const auto& circle : obj.circles) {
      const auto& color = circle.color;
      painter->setPen(QPen(QColor::fromRgbF(color.r, color.g, color.b), 0));
      painter->setBrush(QColor::fromRgbF(color.r, color.g, color.b, 0.4));
      const auto p = vecToPoint(obj.worldPoint(circle.center));
      painter->drawEllipse(p, circle.radius, circle.radius);
      if (lod_size < 5.0) {
        break;
      }
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
      if (lod_size < 8.0) {
        break;
      }
    }
  }

  if (enable_highlighting_ && selection_radius > 0) {
    const auto r = selection_radius * 1.2;
    painter->setPen(QPen(Qt::black, 0, Qt::PenStyle::DashLine));
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(selection_pos, r, r);
  }
}
