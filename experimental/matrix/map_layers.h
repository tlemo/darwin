
#pragma once

#include "visible_world_state.h"

#include <QGraphicsItem>
#include <QRectF>

class MapLayer : public QGraphicsItem {
 public:
  explicit MapLayer(const QRectF& bounding_rect) : bounding_rect_(bounding_rect) {
    setFlag(QGraphicsItem::ItemUsesExtendedStyleOption);
  }

  QRectF boundingRect() const override { return bounding_rect_; }

 protected:
  const QRectF bounding_rect_;
};

class WorldMap : public MapLayer {
 public:
  explicit WorldMap(const QRectF& bounding_rect, const vis::World* world)
      : MapLayer(bounding_rect), world_(world) {}

  void paint(QPainter* painter,
             const QStyleOptionGraphicsItem* option,
             QWidget* widget) override;

 private:
  const vis::World* world_ = nullptr;
};
