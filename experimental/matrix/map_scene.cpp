
#include "map_scene.h"
#include "map_layers.h"
#include "world.h"

#include <QRectF>

MapScene::MapScene() {
  const QRectF extents(
      -World::kWidth / 2, -World::kHeight / 2, World::kWidth, World::kHeight);

  setBackgroundBrush(QBrush(Qt::lightGray));
  setSceneRect(extents);

  auto map = new WorldMap(extents, &visible_world_);
  addItem(map);
}

void MapScene::updateState(const vis::World& visible_world) {
  visible_world_ = visible_world;
  invalidate();
}
