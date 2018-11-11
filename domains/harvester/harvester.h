// Copyright 2018 The Darwin Neuroevolution Framework Authors.
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

#include "world_map.h"

#include <core/darwin.h>

namespace harvester {

void init();

//! Domain: A simple 2D map navigation and resource gathering
//!
//! The robot navigates a randomly generated 2D map, represented as a grid where a cell
//! can be one of the following:
//! - Empty
//! - Food (Green): Reaching the cell provides a fixed health increase
//! - Junk Food (Yellow): Provides a minor health increase
//! - Poison (Red): Fixed health loss
//! - Wall (Gray): Blocks movement
//!
//! The robot starts with an initial health value, then at each step it can perform an
//! action. Each action has a health "cost", and once the health drops to zero the
//! experiment is terminated.
//!
//! ### Inputs
//!
//! The robot's vision is constructed from a number of rays (Config::vision_resolution)
//! fanned within a field of view (Config::vision_fov). For each ray, two input values are
//! calculated (distance and color)
//!
//! Input | Value
//! -----:|------
//!    2i | distance
//!  2i+1 | color
//!
//! ### Outputs
//!
//! Output | Value
//! ------:|------
//!      0 | move (magnitude determines the movement distance)
//!      1 | rotate (magnitude determines the rotation angle)
//!
class Harvester : public darwin::Domain {
 public:
  Harvester();
  bool evaluatePopulation(darwin::Population* population) const override;
  size_t inputs() const override { return inputs_; }
  size_t outputs() const override { return outputs_; }

 private:
  size_t inputs_ = 0;
  size_t outputs_ = 0;
};

class Factory : public darwin::DomainFactory {
  unique_ptr<darwin::Domain> create(const core::PropertySet& config) override;

  unique_ptr<core::PropertySet> defaultConfig(darwin::ComplexityHint hint) const override;
};

}  // namespace harvester
