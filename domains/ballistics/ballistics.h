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

#include <core/darwin.h>
#include <core/properties.h>
#include <third_party/box2d/box2d.h>

namespace ballistics {

//! Ballistics domain configuration
struct Config : public core::PropertySet {
  PROPERTY(gravity, float, 9.8f, "Gravitational acceleration");

  PROPERTY(range_min_x, float, 0.0f, "Min target x coordinate");
  PROPERTY(range_max_x, float, 10.0f, "Max target x coordinate");
  PROPERTY(range_min_y, float, -2.5f, "Min target y coordinate");
  PROPERTY(range_max_y, float, 2.5f, "Max target y coordinate");

  PROPERTY(target_radius, float, 0.1f, "Target radius");
  
  PROPERTY(target_hit_bonus, float, 0.5f, "Extra score for hitting the target [0..1]");

  PROPERTY(projectile_radius, float, 0.1f, "Projectile size");
  PROPERTY(projectile_velocity, float, 10.0f, "Initial projectile velocity");

  PROPERTY(test_worlds, int, 5, "Number of test worlds per generation");
};

//! Domain: Ballistics
//!
//! A basic [ballistics](https://en.wikipedia.org/wiki/Projectile_motion) problem: output
//! the angle required to hit a target at (x, y) coordinates. The magnitude of the initial
//! velocity is fixed.
//!
//! ![](images/ballistics_sandbox.png)
//!
//! While seemingly simple, the analytical solution is not exactly trivial and it seems a 
//! good benchmark for Neuroevolution. The fitness function is:
//! 
//! `1 - closest_distance / target_distance`
//! 
//! An optional fitness bonus is added if the target is hit.
//!
//! ### Inputs
//! 
//! The inputs are simply the (x, y) target coordinates.
//!
//! Input | Value
//! -----:|------
//!     0 | target x coordinate
//!     1 | target y coordinate
//!
//! ### Outputs
//!
//! The single output indicates the aim angle
//!
//! Output | Value
//! ------:|------
//!      0 | aim angle (radians)
//!
class Ballistics : public darwin::Domain {
 public:
  explicit Ballistics(const core::PropertySet& config);

  size_t inputs() const override;
  size_t outputs() const override;

  bool evaluatePopulation(darwin::Population* population) const override;
  
  const Config& config() const { return config_; }
  
  b2Vec2 randomTargetPosition() const;
  
 private:
  void validateConfiguration();

 private:
  Config config_;
};

class Factory : public darwin::DomainFactory {
  unique_ptr<darwin::Domain> create(const core::PropertySet& config) override;
  unique_ptr<core::PropertySet> defaultConfig(darwin::ComplexityHint hint) const override;
};

inline void init() {
  darwin::registry()->domains.add<Factory>("ballistics");
}

}  // namespace ballistics
