
#pragma once

#include <QGraphicsScene>

#include "visible_world_state.h"
#include "map_layers.h"

class MapScene : public QGraphicsScene {
 public:
  MapScene();

  void updateState(const sf::World& visible_world);

 private:
  sf::World visible_world_;
};
