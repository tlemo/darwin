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

namespace ballistics {

Agent::Agent(const darwin::Genotype* genotype) : brain_(genotype->grow()) {}

float Agent::aim(float target_x, float target_y) {
  brain_->setInput(kInputTargetX, target_x);
  brain_->setInput(kInputTargetY, target_y);

  brain_->think();

  return brain_->output(kOutputAimAngle);
}

}  // namespace ballistics
