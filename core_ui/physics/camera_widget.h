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

#include <core/physics/camera.h>

#include <QWidget>

namespace physics_ui {

//! Visualization for a physics::Camera object
class CameraWidget : public QWidget {
 public:
  explicit CameraWidget(QWidget* parent) : QWidget(parent) {}

  void setCamera(const physics::Camera* camera);

 private:
  void paintEvent(QPaintEvent* event) override;

 private:
  const physics::Camera* camera_ = nullptr;
};

}  // namespace physics_ui
