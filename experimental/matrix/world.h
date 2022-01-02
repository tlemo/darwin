
#pragma once

#include "visible_world_state.h"

#include <core/rate_tracker.h>
#include <core/utils.h>
#include <core/sim/scene.h>
#include <third_party/box2d/box2d.h>

#include <mutex>
#include <condition_variable>
#include <atomic>

class World : public core::NonCopyable, public core::PolymorphicBase {
  enum class SimState { Invalid, Paused, Running };

 public:
  explicit World(const sim::Rect& extents);

  void runSimulation();
  void pauseSimulation();

  const vis::World visibleState();

  double ups() const { return ups_; }

  const auto& extents() const { return extents_; }

  float timestamp() const { return timestamp_; }

 private:
  void simThread();
  void simStep();

  vis::World extractVisibleState() const;

 protected:
  b2World world_;
  sim::Rect extents_;
  float timestamp_ = 0;

 private:
  vis::World snapshot_;
  core::RateTracker ups_tracker_;
  std::atomic<double> ups_;
  mutable std::mutex snapshot_lock_;

  SimState sim_state_ = SimState::Invalid;
  mutable std::mutex state_lock_;
  mutable std::condition_variable state_cv_;
};
