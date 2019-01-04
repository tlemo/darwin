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

namespace cart_pole {

Agent::Agent(const darwin::Genotype* genotype, World* world)
    : world_(world), brain_(genotype->grow()) {}

void Agent::simStep() {
  brain_->setInput(0, world_->poleAngle());
  brain_->setInput(1, world_->cartDistance());
  brain_->setInput(2, world_->cartVelocity());
  
  brain_->think();
  
  world_->moveCart(brain_->output(0));
}

}  // namespace cart_pole
