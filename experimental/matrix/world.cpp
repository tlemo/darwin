
#include "world.h"

#include <algorithm>
#include <random>
#include <cmath>
#include <limits>
#include <assert.h>
#include <thread>
using namespace std;

World::World()
    : sim::Scene(b2Vec2(0, 0), sim::Rect(-kWidth / 2, -kHeight / 2, kWidth, kHeight)) {
  new std::thread(&World::simThread, this);
}

bool World::obstructed(const sf::Pos& pos) const {
  for (const auto& obstacle : world_.obstacles) {
    float left = obstacle.pos.x;
    float right = left + obstacle.size.width;
    float top = obstacle.pos.y;
    float bottom = top + obstacle.size.height;

    if (pos.x >= left && pos.x <= right && pos.y >= top && pos.y <= bottom)
      return true;
  }

  return false;
}

void World::generateWorld() {
  printf("Generating world...\n");

  unique_lock<mutex> guard(lock_);
  CHECK(sim_state_ == SimState::Invalid);
  sim_state_ = SimState::Paused;

  random_device rd;
  default_random_engine rnd(rd());
  uniform_real_distribution<float> distX(ROBOT_DIAMETER, WORLD_WIDTH - ROBOT_DIAMETER);
  uniform_real_distribution<float> distY(ROBOT_DIAMETER, WORLD_HEIGHT - ROBOT_DIAMETER);
  uniform_int_distribution<int> distFarmSize(MIN_FARM_SIZE, MAX_FARM_SIZE);
  uniform_real_distribution<float> distObstacleX(0, WORLD_WIDTH - MAX_OBSTACLE_SIZE / 2);
  uniform_real_distribution<float> distObstacleY(0, WORLD_HEIGHT - MAX_OBSTACLE_SIZE / 2);
  uniform_int_distribution<int> distObstacleSize(MIN_OBSTACLE_SIZE, MAX_OBSTACLE_SIZE);
  uniform_real_distribution<float> distAngle(0, 360);

  // farms
  for (int i = 0; i < FARMS_COUNT; ++i) {
    auto& farm = world_.farms[i];
    farm.pos.x = distX(rnd);
    farm.pos.y = distY(rnd);
    farm.size = distFarmSize(rnd);
  }

  // obstacles
  for (int i = 0; i < OBSTACLES_COUNT; ++i) {
    auto& obstacle = world_.obstacles[i];
    obstacle.pos.x = distObstacleX(rnd);
    obstacle.pos.y = distObstacleY(rnd);
    obstacle.size.width = distObstacleSize(rnd);
    obstacle.size.height = distObstacleSize(rnd);
  }

  // food
  for (int i = 0; i < FOOD_COUNT; ++i) {
    auto& food = world_.food[i];
    food.pos.x = distX(rnd);
    food.pos.y = distY(rnd);

    if (obstructed(food.pos))
      --i;
  }

  // junk food
  for (int i = 0; i < JUNK_FOOD_COUNT; ++i) {
    auto& junkFood = world_.junkFood[i];
    junkFood.pos.x = distX(rnd);
    junkFood.pos.y = distY(rnd);

    if (obstructed(junkFood.pos))
      --i;
  }

  // poison
  for (int i = 0; i < POISON_COUNT; ++i) {
    auto& poison = world_.poison[i];
    poison.pos.x = distX(rnd);
    poison.pos.y = distY(rnd);

    if (obstructed(poison.pos))
      --i;
  }

  // robots
  for (int i = 0; i < ROBOTS_COUNT; ++i) {
    auto& robot = world_.robots[i];
    robot.pos.x = distX(rnd);
    robot.pos.y = distY(rnd);
    robot.angle = distAngle(rnd);

    if (obstructed(robot.pos))
      --i;
  }
}

void World::runSimulation() {
  unique_lock<mutex> guard(lock_);
  CHECK(sim_state_ != SimState::Invalid);
  sim_state_ = SimState::Running;
}

void World::pauseSimulation() {
  unique_lock<mutex> guard(lock_);
  CHECK(sim_state_ != SimState::Invalid);
  sim_state_ = SimState::Paused;
}

void World::simThread() {
  for (;;) {
    {
      unique_lock<mutex> guard(lock_);
      while (sim_state_ != SimState::Running) {
        cv_.wait(guard);
      }
    }

    simStep();
  }
}

float World::moveRobot(sf::Robot& robot, float dist) {
  assert(dist > 0);

  const float PI = 3.14159f;
  const float D2R = PI / 180.0f;

  float x = robot.pos.x;
  float y = robot.pos.y;
  float dx = cosf(robot.angle * D2R) * dist;
  float dy = sinf(robot.angle * D2R) * dist;

  // horizontal/vertical normalized collision distance
  // (we're looking for the smallest stictly positive value)
  float ha = numeric_limits<float>::infinity();
  float va = numeric_limits<float>::infinity();

  // segment intersection between (x, y, dx, dy) and (x2, y2, dx2, dy2)
  //
  // NOTE: returns strictly positive values normalized relative to the
  //      (x, y, dx, dy) segment or infinity if there's no intersection
  //
  // this is solved as the solution for the system of 2 equations:
  //   x = x' => x0 + dx * a = x0' + dx' * a'
  //   y = y' => y0 + dy * a = y0' + dy' * a'
  //
  auto intersect2d = [=](float x2, float y2, float dx2, float dy2) {
    // parallel segments?
    if (dy * dx2 - dx * dy2 == 0)
      return numeric_limits<float>::infinity();

    float a = (dx2 * (y2 - y) - dy2 * (x2 - x)) / (dy * dx2 - dx * dy2);

    float b = (dx2 != 0) ? ((x + a * dx) - x2) / dx2 : ((y + a * dy) - y2) / dy2;

    if (a <= 0 || b < 0 || b > 1)
      return numeric_limits<float>::infinity();

    return a;
  };

  // test the world top & bottom edges
  ha = min(ha, intersect2d(0, 0, float(WORLD_WIDTH), 0));
  ha = min(ha, intersect2d(0, float(WORLD_HEIGHT), float(WORLD_WIDTH), 0));

  // test the world left & right edges
  va = min(va, intersect2d(0, 0, 0, float(WORLD_HEIGHT)));
  va = min(va, intersect2d(float(WORLD_WIDTH), 0, 0, float(WORLD_HEIGHT)));

  // test each obstacle in the world
  for (const auto& obstacle : world_.obstacles) {
    float posx = obstacle.pos.x;
    float posy = obstacle.pos.y;
    float width = obstacle.size.width;
    float height = obstacle.size.height;

    // test the top & bottom edges
    ha = min(ha, intersect2d(posx, posy, width, 0));
    ha = min(ha, intersect2d(posx, posy + height, width, 0));

    // test the left & right edges
    va = min(va, intersect2d(posx, posy, 0, height));
    va = min(va, intersect2d(posx + width, posy, 0, height));
  }

  assert(ha > 0);
  assert(va > 0);

  if (min(ha, va) <= 1) {
    const float SAFE_DIST = 0.1f;

    if (ha < va) {
      // collision with a horizontal segment -> vertical bounce
      robot.pos.x += dx * (ha - SAFE_DIST);
      robot.pos.y += dy * (ha - SAFE_DIST);
      robot.angle = 360 - robot.angle;
      return dist * (1 - ha);
    } else {
      // collision with a vertical segment -> horizontal bounce
      robot.pos.x += dx * (va - SAFE_DIST);
      robot.pos.y += dy * (va - SAFE_DIST);
      robot.angle = 180 - robot.angle;
      return dist * (1 - va);
    }
  }

  // no obstructions
  robot.pos.x += dx;
  robot.pos.y += dy;
  return 0;
}

void World::simStep() {
  for (int i = 0; i < ROBOTS_COUNT; ++i) {
    auto& robot = world_.robots[i];
    float dist = 2.0f;
    int bounces = 0;
    while (dist > 0) {
      assert(bounces < 4);
      dist = moveRobot(robot, dist);
      ++bounces;
    }
  }
}
