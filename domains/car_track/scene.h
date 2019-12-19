// Copyright 2019 The Darwin Neuroevolution Framework Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "domain.h"

#include <core/sim/scene.h>
#include <core/sim/drone.h>
#include <core/sim/track.h>
#include <core/properties.h>

#include <memory>
using namespace std;

namespace car_track {

using sim::Drone;

struct SceneVariables : public core::PropertySet {
  PROPERTY(drone_x, float, 0, "Drone x coordinate");
  PROPERTY(drone_y, float, 0, "Drone y coordinate");
  PROPERTY(drone_vx, float, 0, "Drone velocity (x component)");
  PROPERTY(drone_vy, float, 0, "Drone velocity (y component)");
  PROPERTY(drone_dir, float, 0, "Heading angle");
  PROPERTY(distance, int, 0, "Track distance from the start (one lap is 1.0)");
};

class Scene : public sim::Scene {
 public:
  static constexpr float kWidth = 50;
  static constexpr float kHeight = 20;

 public:
  explicit Scene(const sim::Track* track, const CarTrack* domain);

  const SceneVariables* variables() const override { return &variables_; }

  const Config* config() const override { return &domain_->config(); }

  Drone* drone() { return drone_.get(); }

  const sim::Track* track() const { return track_; }

  const CarTrack* domain() const { return domain_; }

  //! returns the current fitness value
  float fitness() const;

  void postStep(float dt) override;

 private:
  void updateVariables();
  void updateFitness();

 private:
  float fitness_ = 0;
  int distance_ = 0;
  unique_ptr<Drone> drone_;

  SceneVariables variables_;
  const sim::Track* track_ = nullptr;
  const CarTrack* domain_ = nullptr;
};

}  // namespace car_track
