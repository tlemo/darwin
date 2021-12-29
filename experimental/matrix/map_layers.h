
#pragma once

#include "visible_world_state.h"

#include <QGraphicsItem>
#include <QRectF>

class MapLayer : public QGraphicsItem {
 public:
  explicit MapLayer(const sf::World* state) : state_(state) {
    setFlag(QGraphicsItem::ItemUsesExtendedStyleOption);
  }

  QRectF boundingRect() const override { return QRectF(0, 0, WORLD_WIDTH, WORLD_HEIGHT); }

 protected:
  const sf::World* state_ = nullptr;
};

class WorldMap : public MapLayer {
 public:
  explicit WorldMap(const sf::World* state) : MapLayer(state) {
    setFlag(QGraphicsItem::ItemClipsChildrenToShape);
  }

  void paint(QPainter* painter,
             const QStyleOptionGraphicsItem* option,
             QWidget* widget) override;
};

class RobotsLayer : public MapLayer {
 public:
  explicit RobotsLayer(const sf::World* state) : MapLayer(state) {}
  void paint(QPainter* painter,
             const QStyleOptionGraphicsItem* option,
             QWidget* widget) override;
};

class FoodLayer : public MapLayer {
 public:
  explicit FoodLayer(const sf::World* state) : MapLayer(state) {}
  void paint(QPainter* painter,
             const QStyleOptionGraphicsItem* option,
             QWidget* widget) override;
};

class JunkFoodLayer : public MapLayer {
 public:
  explicit JunkFoodLayer(const sf::World* state) : MapLayer(state) {}
  void paint(QPainter* painter,
             const QStyleOptionGraphicsItem* option,
             QWidget* widget) override;
};

class PoisonLayer : public MapLayer {
 public:
  explicit PoisonLayer(const sf::World* state) : MapLayer(state) {}
  void paint(QPainter* painter,
             const QStyleOptionGraphicsItem* option,
             QWidget* widget) override;
};

class ObstaclesLayer : public MapLayer {
 public:
  explicit ObstaclesLayer(const sf::World* state) : MapLayer(state) {}
  void paint(QPainter* painter,
             const QStyleOptionGraphicsItem* option,
             QWidget* widget) override;
};

class FarmsLayer : public MapLayer {
 public:
  explicit FarmsLayer(const sf::World* state) : MapLayer(state) {}
  void paint(QPainter* painter,
             const QStyleOptionGraphicsItem* option,
             QWidget* widget) override;
};
