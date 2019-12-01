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
#include "track.h"

#include <core/sim/scene.h>
#include <core/sim/drone.h>
#include <core/properties.h>

#include <memory>
#include <random>
using namespace std;

namespace drone_track {

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
  static constexpr float kWidth = 50;
  static constexpr float kHeight = 20;

 public:
  using Seed = std::random_device::result_type;

 public:
  explicit Scene(Seed seed, const DroneTrack* domain);

  const SceneVariables* variables() const override { return &variables_; }

  const Config* config() const override { return &domain_->config(); }

  Drone* drone() { return drone_.get(); }
  
  const Track* track() const { return track_.get(); }

  const DroneTrack* domain() const { return domain_; }

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
  unique_ptr<Track> track_;

  default_random_engine rnd_;

  SceneVariables variables_;
  const DroneTrack* domain_ = nullptr;
};

}  // namespace drone_track
