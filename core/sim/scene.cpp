// Copyright The Darwin Neuroevolution Framework Authors.
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

#include "scene.h"

namespace sim {

bool Scene::simStep() {
  if (timestamp_ == 0) {
    script_.start();
  }

  preStep();

  constexpr float32 time_step = 1.0f / 50.0f;
  constexpr int32 velocity_iterations = 10;
  constexpr int32 position_iterations = 10;

  script_.play(timestamp_);

  // Box2D simulation step
  world_.Step(time_step, velocity_iterations, position_iterations);
  timestamp_ += time_step;

  // TODO: process the contacts
  // TODO: pause/resume/done/reset?

  postStep(time_step);
  return true;
}

}  // namespace sim
