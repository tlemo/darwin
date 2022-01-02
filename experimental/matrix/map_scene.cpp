
#include "map_scene.h"
#include "world.h"

#include <QRectF>

MapScene::MapScene() {
  const auto sim_extents = world_.extents();
  const QRectF extents(
      sim_extents.x, sim_extents.y, sim_extents.width, sim_extents.height);

  setBackgroundBrush(QBrush(Qt::lightGray));
  setSceneRect(extents);

  world_layer_ = new WorldMap(extents, &visible_world_);
  addItem(world_layer_);
}

void MapScene::setCursorPosition(const QPointF& pos, bool enable_highlighting) {
  world_layer_->setCursorPosition(pos, enable_highlighting);
}

void MapScene::updateScene() {
  visible_world_ = world_.visibleState();
  invalidate();
}
