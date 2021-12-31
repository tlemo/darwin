
#pragma once

#include "map_layers.h"
#include "visible_world_state.h"

#include <QGraphicsScene>
#include <QPointF>

class MapScene : public QGraphicsScene {
 public:
  MapScene();

  void setCursorPosition(const QPointF& pos, bool enable_highlighting);

  void updateState(const vis::World& visible_world);

 private:
  vis::World visible_world_;
  WorldMap* world_layer_ = nullptr;
};
