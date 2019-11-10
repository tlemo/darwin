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

#include <core/sim/accelerometer.h>
#include <core_ui/canvas.h>

namespace physics_ui {

//! Visualization for a sim::Accelerometer object
class AccelerometerWidget : public core_ui::Canvas {
  static constexpr double kCanvasWidth = 20;
  static constexpr double kCanvasHeight = 20;

  static constexpr double kSensorWidth = 15;
  static constexpr double kSensorHeight = 15;

  static constexpr double kSkinSize = 0.7;
  static constexpr double kVectorLength = 6;
  static constexpr double kVectorWidth = 0.4;

 public:
  explicit AccelerometerWidget(QWidget* parent);

  void setSensor(const sim::Accelerometer* sensor);

 private:
  void paintEvent(QPaintEvent* event) override;

 private:
  const sim::Accelerometer* sensor_ = nullptr;
};

}  // namespace physics_ui
