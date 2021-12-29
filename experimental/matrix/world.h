
#pragma once

#include "visible_world_state.h"

#include <core/sim/scene.h>

#include <mutex>
#include <condition_variable>

class World : public sim::Scene {
  static constexpr float kWidth = 200;
  static constexpr float kHeight = 100;

  enum class SimState { Invalid, Paused, Running };

 public:
  World();

  void generateWorld();

  void runSimulation();
  void pauseSimulation();

  const auto& visibleState() const { return world_; }

 private:
  void simThread();
  void simStep();
  bool obstructed(const sf::Pos& pos) const;
  float moveRobot(sf::Robot& robot, float dist);

 private:
  sf::World world_;

  SimState sim_state_ = SimState::Invalid;

  std::mutex lock_;
  std::condition_variable cv_;
};
