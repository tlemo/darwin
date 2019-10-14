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

#pragma once

#include <core/physics/compass.h>
#include <core_ui/canvas.h>

namespace physics_ui {

//! Visualization for a physics::Compass object
class CompassWidget : public core_ui::Canvas {
  static constexpr double kCanvasWidth = 20;
  static constexpr double kCanvasHeight = 20;

  static constexpr double kSensorWidth = 15;
  static constexpr double kSensorHeight = 15;

  static constexpr double kCompassLength = 7;
  static constexpr double kCompassWidth = 0.4;

 public:
  explicit CompassWidget(QWidget* parent);

  void setSensor(const physics::Compass* sensor);

 private:
  void paintEvent(QPaintEvent* event) override;

 private:
  const physics::Compass* sensor_ = nullptr;
};

}  // namespace physics_ui
