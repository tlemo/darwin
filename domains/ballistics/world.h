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

#include "ballistics.h"

#include <third_party/box2d/box2d.h>

namespace ballistics {

class World {
 public:
  World(b2Vec2 target_pos, const Ballistics* domain);

  // creates a projectile with the initial velocity and the given angle (radians)
  void fireProjectile(float aim_angle);
  
  // advances the physical simulation one step, returning false
  // if the state reaches one of the termination conditions
  bool simStep();
  
  // world features
  b2Vec2 projectilePosition() const { return projectile_->GetPosition(); }
  b2Vec2 targetPosition() const { return target_->GetPosition(); }
  
  float verticalLimit() const { return vertical_limit_; }
  void setVerticalLimit(float y) { vertical_limit_ = y; }

  const Ballistics* domain() const { return domain_; }

  b2World* box2dWorld() { return &b2_world_; }
 
 private:
  b2World b2_world_;
  b2Body* target_ = nullptr;
  b2Body* projectile_ = nullptr;
  
  float vertical_limit_ = 0;
  
  const Ballistics* domain_ = nullptr;
};

}  // namespace ballistics
