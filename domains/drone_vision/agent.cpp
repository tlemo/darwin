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

#include "agent.h"
#include "scene.h"

namespace drone_vision {

Agent::Agent(const darwin::Genotype* genotype, Scene* scene)
    : scene_(scene), brain_(genotype->grow()) {}

void Agent::simStep() {
  const auto& config = scene_->domain()->config();

#if 0 // TODO
  // setup inputs
  int input_index = 0;
  if (config.input_pole_angle)
    brain_->setInput(input_index++, scene_->poleAngle());
  if (config.input_angular_velocity)
    brain_->setInput(input_index++, scene_->poleAngularVelocity());
  if (config.input_wheel_distance)
    brain_->setInput(input_index++, scene_->wheelDistance());
  if (config.input_wheel_velocity)
    brain_->setInput(input_index++, scene_->wheelVelocity());
  if (config.input_distance_from_target)
    brain_->setInput(input_index++, scene_->wheelDistance() - scene_->targetPosition());

  brain_->think();

  // act based on the output values
  scene_->turnWheel(brain_->output(0));
#endif
}

int Agent::inputs(const Config& config) {
  int inputs_count = 0;
#if 0 // TODO
  if (config.input_pole_angle)
    ++inputs_count;
  if (config.input_angular_velocity)
    ++inputs_count;
  if (config.input_wheel_distance)
    ++inputs_count;
  if (config.input_wheel_velocity)
    ++inputs_count;
  if (config.input_distance_from_target)
    ++inputs_count;
#endif
  return inputs_count;
}

int Agent::outputs(const Config&) {
  return 1;
}

}  // namespace drone_vision
