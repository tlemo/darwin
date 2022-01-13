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
#include <core/properties.h>
#include <core/random.h>

#include <memory>
#include <random>
using namespace std;

namespace drone_vision {

using sim::Drone;

struct SceneVariables : public core::PropertySet {
  PROPERTY(drone_x, float, 0, "Drone x coordinate");
  PROPERTY(drone_y, float, 0, "Drone y coordinate");
  PROPERTY(drone_vx, float, 0, "Drone velocity (x component)");
  PROPERTY(drone_vy, float, 0, "Drone velocity (y component)");
  PROPERTY(drone_dir, float, 0, "Heading angle");
  PROPERTY(target_angle, float, 0, "Angle between drone direction and target");
};

class Scene : public sim::Scene {
 public:
  explicit Scene(const b2Vec2& target_velocity, const DroneVision* domain);

  const SceneVariables* variables() const override { return &variables_; }

  const Config* config() const override { return &domain_->config(); }

  Drone* drone() { return drone_.get(); }

  const DroneVision* domain() const { return domain_; }

  //! returns the current fitness value
  float fitness() const { return fitness_; }

  void postStep(float dt) override;

 private:
  b2Body* createTarget(const b2Vec2& pos, const b2Vec2& v, float radius);
  void createDebris();
  void createRoundDebris(const b2Vec2& pos, float radius);
  void createBoxDebris(const b2Vec2& pos, float width, float height);
  void createColumns();
  void createColumnFixture(b2Body* body, const b2Vec2& pos, const b2Color& color);
  void createLight(b2Body* body, const b2Vec2& pos, const b2Color& color);
  void updateFitness();
  void updateVariables();

  //! returns the angle between the drone direction and the target
  float aimAngle() const;

 private:
  b2Body* target_ = nullptr;
  float fitness_ = 0;
  unique_ptr<Drone> drone_;
  SceneVariables variables_;
  const DroneVision* domain_ = nullptr;
  default_random_engine rnd_{ core::randomSeed() };
};

}  // namespace drone_vision
