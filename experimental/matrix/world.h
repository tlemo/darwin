
#pragma once

#include "visible_world_state.h"

#include <core/rate_tracker.h>
#include <core/utils.h>
#include <core/sim/scene.h>
#include <third_party/box2d/box2d.h>

#include <QImage>
#include <QTransform>

#include <mutex>
#include <condition_variable>
#include <atomic>

// This looks very similar to sim::Scene, with a few important differences:
// - The simulation runs on a dedicated background thread
// - The visible state is extracted and made available thread-safe
class World : public core::NonCopyable, public core::PolymorphicBase {
  enum class SimState { Invalid, Paused, Running };

  static constexpr double kVisualMapScale = 10.0;

 public:
  explicit World(const sim::Rect& extents);

  void runSimulation();
  void pauseSimulation();

  const vis::World visibleState();

  // experimental!
  const QImage* visualMap() const { return &visual_map_; }

  double ups() const { return ups_; }

  const auto& extents() const { return extents_; }

  float timestamp() const { return timestamp_; }

  virtual void preStep() {}
  virtual void postStep(float /*dt*/) {}

  b2World* box2dWorld() { return &world_; }

 private:
  void simThread();
  void simStep();

  void extractVisibleState();

  void setupVisualMapTransformation();

  // updates visual_map_, called from the simulation thread
  void renderVisualMap();

 protected:
  b2World world_;
  sim::Rect extents_;
  float timestamp_ = 0;

 private:
  vis::World snapshot_;
  vis::World snapshot_staging_;
  mutable std::mutex snapshot_lock_;

  core::RateTracker ups_tracker_;
  std::atomic<double> ups_;

  SimState sim_state_ = SimState::Invalid;
  mutable std::mutex state_lock_;
  mutable std::condition_variable state_cv_;

  QImage visual_map_;
  QTransform visual_map_transformation_;
};
