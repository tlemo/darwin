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

#include "car_controller.h"

namespace sim {

CarController::CarController(const darwin::Genotype* genotype, Car* car)
    : car_(car), brain_(genotype->grow()) {}

void CarController::simStep() {
  const auto& config = car_->config();

  // setup inputs
  int input_index = 0;

  if (config.camera) {
    const auto camera = car_->camera();
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
    const auto receptors = car_->touchSensor()->receptors();
    for (float value : receptors) {
      brain_->setInput(input_index++, value);
    }
  }

  if (config.compass) {
    const auto& heading = car_->compass()->heading();
    brain_->setInput(input_index++, heading.x);
    brain_->setInput(input_index++, heading.y);
  }

  if (config.accelerometer) {
    const auto linear_acc = car_->accelerometer()->linearAcceleration();
    const auto angular_acc = car_->accelerometer()->angularAcceleration();
    brain_->setInput(input_index++, linear_acc.x);
    brain_->setInput(input_index++, linear_acc.y);
    brain_->setInput(input_index++, angular_acc);
  }

  // think!
  brain_->think();

  const float acceleration_force = brain_->output(0);
  CHECK(!isnan(acceleration_force));
  car_->accelerate(acceleration_force);

  const float steer = brain_->output(1);
  CHECK(!isnan(steer));
  car_->steer(steer);
}

int CarController::inputs(const CarConfig& config) {
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

int CarController::outputs(const CarConfig&) {
  // outputs:
  //  0: acceleration
  //  1: steering
  return 2;
}

}  // namespace sim
