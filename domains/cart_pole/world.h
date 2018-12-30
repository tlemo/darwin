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

#include "cart_pole.h"

#include <third_party/box2d/box2d.h>

namespace cart_pole {

class World {
 public:
  World(float start_angle, const Config& config);
  
  // advances the physical simulation one step, returning false
  // if the state reaches one of the termination conditions
  bool simStep();
  
 private:
  b2World b2_world_;
  const Config& config_;
};

}  // namespace cart_pole
