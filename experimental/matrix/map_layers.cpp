
#include "map_layers.h"

#include <QPen>
#include <QBrush>
#include <QPainter>
#include <QPolygon>
#include <QRectF>
#include <QMarginsF>
#include <QStyleOptionGraphicsItem>

void WorldMap::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
  painter->setBrush(QBrush(Qt::white));
  painter->setPen(Qt::NoPen);
  painter->drawRect(0, 0, WORLD_WIDTH, WORLD_HEIGHT);
}

void RobotsLayer::paint(QPainter* painter,
                        const QStyleOptionGraphicsItem* option,
                        QWidget*) {
  const int ds = ROBOT_DIAMETER;
  auto lod = option->levelOfDetailFromTransform(painter->worldTransform());
  auto clipRect = option->exposedRect.adjusted(-ds, -ds, ds, ds);

  auto robotColor = QColor(128, 128, 255);
  painter->setBrush(robotColor);

  if (lod > 0.3)
    painter->setPen(QPen(Qt::black, 0.8));
  else if (lod > 0.2)
    painter->setPen(QPen(robotColor, 8));
  else
    painter->setPen(QPen(robotColor, 12));

  for (int i = 0; i < ROBOTS_COUNT; ++i) {
    auto& robot = state_->robots[i];
    float x = robot.pos.x;
    float y = robot.pos.y;

    if (!clipRect.contains(x, y))
      continue;

    if (lod > 0.3) {
      float pieAngle = 180 - robot.angle;
      QRectF pieRect(x - ds, y - ds, 2 * ds, 2 * ds);
      painter->drawPie(pieRect, (pieAngle - 20) * 16, 40 * 16);
    } else {
      painter->drawPoint(x, y);
    }
  }
}

void FoodLayer::paint(QPainter* painter,
                      const QStyleOptionGraphicsItem* option,
                      QWidget*) {
  const int ds = FOOD_DIAMETER;
  auto lod = option->levelOfDetailFromTransform(painter->worldTransform());
  auto clipRect = option->exposedRect.adjusted(-ds, -ds, ds, ds);

  if (lod < 0.1)
    return;

  painter->setBrush(Qt::green);
  painter->setPen(Qt::NoPen);

  for (int i = 0; i < FOOD_COUNT; ++i) {
    auto& food = state_->food[i];
    auto x = food.pos.x;
    auto y = food.pos.y;

    if (!clipRect.contains(x, y))
      continue;

    painter->drawEllipse(x - ds, y - ds, 2 * ds, 2 * ds);
  }
}

void JunkFoodLayer::paint(QPainter* painter,
                          const QStyleOptionGraphicsItem* option,
                          QWidget*) {
  const int ds = FOOD_DIAMETER;
  auto lod = option->levelOfDetailFromTransform(painter->worldTransform());
  auto clipRect = option->exposedRect.adjusted(-ds, -ds, ds, ds);

  if (lod < 0.1)
    return;

  painter->setBrush(QColor(200, 200, 0));
  painter->setPen(Qt::NoPen);

  for (int i = 0; i < JUNK_FOOD_COUNT; ++i) {
    auto& junkFood = state_->junkFood[i];
    auto x = junkFood.pos.x;
    auto y = junkFood.pos.y;

    if (!clipRect.contains(x, y))
      continue;

    painter->drawEllipse(x - ds, y - ds, 2 * ds, 2 * ds);
  }
}

void PoisonLayer::paint(QPainter* painter,
                        const QStyleOptionGraphicsItem* option,
                        QWidget*) {
  const int ds = FOOD_DIAMETER;
  auto lod = option->levelOfDetailFromTransform(painter->worldTransform());
  auto clipRect = option->exposedRect.adjusted(-ds, -ds, ds, ds);

  if (lod < 0.1)
    return;

  painter->setBrush(Qt::red);
  painter->setPen(Qt::NoPen);

  for (int i = 0; i < POISON_COUNT; ++i) {
    auto& poison = state_->poison[i];
    auto x = poison.pos.x;
    auto y = poison.pos.y;

    if (!clipRect.contains(x, y))
      continue;

    painter->drawEllipse(x - ds, y - ds, 2 * ds, 2 * ds);
  }
}

void FarmsLayer::paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget*) {
  auto& clipRect = option->exposedRect;

  painter->setBrush(QColor(208, 255, 208));
  painter->setPen(Qt::NoPen);

  for (int i = 0; i < FARMS_COUNT; ++i) {
    auto& farm = state_->farms[i];
    auto x = farm.pos.x;
    auto y = farm.pos.y;
    auto size = farm.size;
    QRectF rect(x - size, y - size, size * 2, size * 2);

    if (!clipRect.intersects(rect))
      continue;

    painter->drawEllipse(rect);
  }
}

void ObstaclesLayer::paint(QPainter* painter,
                           const QStyleOptionGraphicsItem* option,
                           QWidget*) {
  auto& clipRect = option->exposedRect;

  painter->setBrush(QColor(16, 16, 64));
  painter->setPen(Qt::NoPen);

  for (int i = 0; i < OBSTACLES_COUNT; ++i) {
    auto& obstacle = state_->obstacles[i];
    auto x = obstacle.pos.x;
    auto y = obstacle.pos.y;
    auto width = obstacle.size.width;
    auto height = obstacle.size.height;
    QRectF rect(x, y, width, height);

    if (!clipRect.intersects(rect))
      continue;

    painter->drawRect(rect);
  }
}
