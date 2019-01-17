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
#include "world.h"

namespace unicycle {

Agent::Agent(const darwin::Genotype* genotype, World* world)
    : world_(world), brain_(genotype->grow()) {}

void Agent::simStep() {
  const auto& config = world_->domain()->config();

  // setup inputs
  int input_index = 0;
  if (config.input_pole_angle)
    brain_->setInput(input_index++, world_->poleAngle());
  if (config.input_angular_velocity)
    brain_->setInput(input_index++, world_->poleAngularVelocity());
  if (config.input_wheel_distance)
    brain_->setInput(input_index++, world_->wheelDistance());
  if (config.input_wheel_velocity)
    brain_->setInput(input_index++, world_->wheelVelocity());
  if (config.input_distance_from_target)
    brain_->setInput(input_index++, world_->wheelDistance() - world_->targetPosition());

  brain_->think();

  // act based on the output values
  world_->turnWheel(brain_->output(0));
}

int Agent::inputs(const Config& config) {
  int inputs_count = 0;
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
  return inputs_count;
}

int Agent::outputs(const Config&) {
  return 1;
}

}  // namespace unicycle
