
#pragma once

#include "map_layers.h"
#include "test_world.h"
#include "seg_tree_world.h"
#include "visible_world_state.h"

#include <QGraphicsScene>
#include <QPointF>

class MapScene : public QGraphicsScene {
  using WorldType = seg_tree::World;

 public:
  MapScene();

  void setCursorPosition(const QPointF& pos, bool enable_highlighting);

  auto& world() { return world_; }
  auto& world() const { return world_; }

  void updateScene();

 private:
  WorldType world_;
  vis::World visible_world_;
  WorldMap* world_layer_ = nullptr;
};
