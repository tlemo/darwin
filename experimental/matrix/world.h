
#pragma once

#include "visible_world_state.h"

#include <core/rate_tracker.h>
#include <core/utils.h>
#include <third_party/box2d/box2d.h>

#include <mutex>
#include <condition_variable>
#include <atomic>

class World : public core::NonCopyable {
 public:
  static constexpr float kWidth = 200;
  static constexpr float kHeight = 100;

  enum class SimState { Invalid, Paused, Running };

 public:
  World();

  void generateWorld();

  void runSimulation();
  void pauseSimulation();

  const vis::World visibleState();

  double ups() const { return ups_; }

 private:
  void simThread();
  void simStep();

  vis::World extractVisibleState() const;

  b2Body* addBall(float x, float y);

 private:
  b2World world_;
  float timestamp_ = 0;

  vis::World snapshot_;
  core::RateTracker ups_tracker_;
  std::atomic<double> ups_;
  mutable std::mutex snapshot_lock_;

  SimState sim_state_ = SimState::Invalid;
  mutable std::mutex state_lock_;
  mutable std::condition_variable state_cv_;
};
