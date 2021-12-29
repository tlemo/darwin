
#include "map_scene.h"

MapScene::MapScene() {
  setBackgroundBrush(QBrush(Qt::lightGray));
  setSceneRect(0, 0, WORLD_WIDTH, WORLD_HEIGHT);

  // world boundary
  auto map = new WorldMap(&visible_world_);
  addItem(map);

  // farms
  auto farmsLayer = new FarmsLayer(&visible_world_);
  farmsLayer->setParentItem(map);

  // obstacles
  auto obstaclesLayer = new ObstaclesLayer(&visible_world_);
  obstaclesLayer->setParentItem(map);

  // food
  auto foodLayer = new FoodLayer(&visible_world_);
  foodLayer->setParentItem(map);

  // junk food
  auto junkFoodLayer = new JunkFoodLayer(&visible_world_);
  junkFoodLayer->setParentItem(map);

  // poison
  auto poisonLayer = new PoisonLayer(&visible_world_);
  poisonLayer->setParentItem(map);

  // robots
  auto robotsLayer = new RobotsLayer(&visible_world_);
  robotsLayer->setParentItem(map);
}

void MapScene::updateState(const sf::World& visible_world) {
  visible_world_ = visible_world;
  invalidate();
}
