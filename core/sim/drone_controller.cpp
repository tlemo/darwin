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

namespace sim {

DroneController::DroneController(const darwin::Genotype* genotype, Drone* drone)
    : drone_(drone), brain_(genotype->grow()) {}

void DroneController::simStep() {
  const auto& config = drone_->config();

  // setup inputs
  int input_index = 0;

  if (config.camera) {
    const auto camera = drone_->camera();
    const auto image = camera->render();
    CHECK(image.size() == config.camera_resolution);
    for (const auto& receptor : image) {
      brain_->setInput(input_index++, receptor.color.r);
      brain_->setInput(input_index++, receptor.color.g);
      brain_->setInput(input_index++, receptor.color.b);
      if (config.camera_depth) {
        brain_->setInput(input_index++, receptor.depth);
      }
    }
  }

  if (config.touch_sensor) {
    const auto receptors = drone_->touchSensor()->receptors();
    for (float value : receptors) {
      brain_->setInput(input_index++, value);
    }
  }

  if (config.compass) {
    const auto& heading = drone_->compass()->heading();
    brain_->setInput(input_index++, heading.x);
    brain_->setInput(input_index++, heading.y);
  }

  if (config.accelerometer) {
    const auto linear_acc = drone_->accelerometer()->linearAcceleration();
    const auto angular_acc = drone_->accelerometer()->angularAcceleration();
    brain_->setInput(input_index++, linear_acc.x);
    brain_->setInput(input_index++, linear_acc.y);
    brain_->setInput(input_index++, angular_acc);
  }

  // think!
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
  int inputs = 0;

  if (config.camera) {
    inputs += config.camera_resolution * (config.camera_depth ? 4 : 3);
  }

  if (config.touch_sensor) {
    inputs += config.touch_resolution;
  }

  if (config.compass) {
    inputs += 2;
  }

  if (config.accelerometer) {
    inputs += 3;
  }

  CHECK(inputs > 0);
  return inputs;
}

int DroneController::outputs(const DroneConfig&) {
  // outputs:
  //  0: move_force_x
  //  1: move_force_y
  //  2: rotate_torque
  return 3;
}

}  // namespace sim
