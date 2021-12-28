
#pragma once

#include "visible_world_state.h"

#include <core/sim/scene.h>

class World : public sim::Scene {
  static constexpr float kWidth = 200;
  static constexpr float kHeight = 100;

 public:
  World();

  void generateWorld();

  void simStep();

  const auto& visibleState() const { return world_; }

 private:
  bool obstructed(const sf::Pos& pos) const;
  float moveRobot(sf::Robot &robot, float dist);

 private:
  sf::World world_;
};
