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

#include <core/sim/camera.h>
#include <core/sim/scene.h>
#include <core/sim/drone.h>
#include <core/properties.h>

#include <memory>
#include <random>
using namespace std;

namespace drone_track {

using sim::Camera;
using sim::Drone;

struct SceneVariables : public core::PropertySet {
  PROPERTY(drone_x, float, 0, "Drone x coordinate");
  PROPERTY(drone_y, float, 0, "Drone y coordinate");
  PROPERTY(drone_vx, float, 0, "Drone velocity (x component)");
  PROPERTY(drone_vy, float, 0, "Drone velocity (y component)");
  PROPERTY(drone_dir, float, 0, "Heading angle");
  PROPERTY(target_dist, float, 0, "Distance from target");
};

class Scene : public sim::Scene {
  static constexpr float kWidth = 20;
  static constexpr float kHeight = 20;

 public:
  using Seed = std::random_device::result_type;

 public:
  explicit Scene(Seed seed, const DroneFollow* domain);

  const SceneVariables* variables() const override { return &variables_; }

  const Config* config() const override { return &domain_->config(); }

  Drone* drone() { return drone_.get(); }
  Drone* targetDrone() { return target_drone_.get(); }

  const DroneFollow* domain() const { return domain_; }

  //! returns the current fitness value
  float fitness() const { return fitness_; }

  void preStep() override;
  void postStep(float dt) override;

 private:
  unique_ptr<Drone> createTargetDrone();
  void createLight(b2Body* body, const b2Vec2& pos, const b2Color& color);
  void createDebris();
  void createRoundDebris(const b2Vec2& pos, float radius);
  void createBoxDebris(const b2Vec2& pos, float width, float height);
  void createColumns();
  void createColumnFixture(b2Body* body, const b2Vec2& pos, const b2Color& color);
  void updateVariables();
  float targetDistance() const;
  void updateFitness();
  void generateTargetPos();

 private:
  float fitness_ = 0;
  unique_ptr<Drone> drone_;

  default_random_engine rnd_;
  unique_ptr<Drone> target_drone_;
  b2Vec2 start_pos_;
  b2Vec2 target_pos_;

  SceneVariables variables_;
  const DroneFollow* domain_ = nullptr;
};

}  // namespace drone_track
