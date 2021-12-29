
#pragma once

#include <stdint.h>

const int WORLD_WIDTH = 40000;
const int WORLD_HEIGHT = 20000;

const int MIN_FARM_SIZE = 150;
const int MAX_FARM_SIZE = 1500;

const int MIN_OBSTACLE_SIZE = 150;
const int MAX_OBSTACLE_SIZE = 1500;

const int FARMS_COUNT = 100;
const int OBSTACLES_COUNT = 1000;
const int FOOD_COUNT = 2000;
const int JUNK_FOOD_COUNT = 2000;
const int POISON_COUNT = 2000;
const int ROBOTS_COUNT = 10000;

const int DEFAULT_ZOOM = 15;

const float ROBOT_DIAMETER = 10.0f;
const float FOOD_DIAMETER = 5.0f;

namespace sf {

struct Pos {
  float x;
  float y;
};

struct Size {
  uint16_t width;
  uint16_t height;
};

struct Robot {
  Pos pos;
  float angle;
};

struct Food {
  Pos pos;
};

struct JunkFood {
  Pos pos;
};

struct Poison {
  Pos pos;
};

struct Obstacle {
  Pos pos;
  Size size;
};

struct Farm {
  Pos pos;
  uint16_t size;
};

struct World {
  Obstacle obstacles[OBSTACLES_COUNT];
  Farm farms[FARMS_COUNT];
  Food food[FOOD_COUNT];
  JunkFood junkFood[JUNK_FOOD_COUNT];
  Poison poison[POISON_COUNT];
  Robot robots[ROBOTS_COUNT];
  double ups = 0;  // updates per second
};

}  // namespace sf
