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

#include "drone_controller.h"

namespace physics {

DroneController::DroneController(const darwin::Genotype* genotype, Drone* drone)
    : drone_(drone), brain_(genotype->grow()) {}

void DroneController::simStep() {
  const auto& config = drone_->config();

  // setup inputs
  const auto camera = drone_->camera();
  const auto image = camera->render();
  CHECK(image.size() == config.camera_resolution);
  int input_index = 0;
  for (const auto& receptor : image) {
    brain_->setInput(input_index++, receptor.color.r);
    brain_->setInput(input_index++, receptor.color.g);
    brain_->setInput(input_index++, receptor.color.b);
    if (config.camera_depth) {
      brain_->setInput(input_index++, receptor.depth);
    }
  }

  brain_->think();

  const float force_x = brain_->output(0);
  const float force_y = brain_->output(1);
  CHECK(!isnan(force_x));
  CHECK(!isnan(force_y));
  drone_->move(b2Vec2(force_x, force_y));

  const float torque = brain_->output(2);
  CHECK(!isnan(torque));
  drone_->rotate(torque);
}

int DroneController::inputs(const DroneConfig& config) {
  // TODO: add support for the rest of the drone sensors
  CHECK(!config.compass);
  CHECK(!config.touch_sensor);
  CHECK(!config.accelerometer);
  return config.camera_resolution * (config.camera_depth ? 4 : 3);
}

int DroneController::outputs(const DroneConfig&) {
  // outputs:
  //  0: move_force_x
  //  1: move_force_y
  //  2: rotate_torque
  return 3;
}

}  // namespace physics
