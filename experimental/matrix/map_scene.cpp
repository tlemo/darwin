
#include "map_scene.h"
#include "world.h"

#include <QRectF>

MapScene::MapScene() {
  const QRectF extents(
      -World::kWidth / 2, -World::kHeight / 2, World::kWidth, World::kHeight);

  setBackgroundBrush(QBrush(Qt::lightGray));
  setSceneRect(extents);

  world_layer_ = new WorldMap(extents, &visible_world_);
  addItem(world_layer_);
}

void MapScene::setCursorPosition(const QPointF& pos, bool enable_highlighting) {
  world_layer_->setCursorPosition(pos, enable_highlighting);
}

void MapScene::updateState(const vis::World& visible_world) {
  visible_world_ = visible_world;
  invalidate();
}
