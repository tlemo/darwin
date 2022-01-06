
#pragma once

#include "world.h"

#include <third_party/box2d/box2d.h>

class SegTreeWorld : public World {
  static constexpr float kWidth = 200;
  static constexpr float kHeight = 100;

 public:
  SegTreeWorld();

 private:
  b2Body* addRobot(const b2Vec2& pos);
};
