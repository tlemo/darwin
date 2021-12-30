
#pragma once

#include <QGraphicsScene>

#include "visible_world_state.h"

class MapScene : public QGraphicsScene {
 public:
  MapScene();

  void updateState(const vis::World& visible_world);

 private:
  vis::World visible_world_;
};
