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

#pragma once

#include <core/sim/drone.h>
#include <core_ui/sim/box2d_widget.h>
#include <domains/car_track/scene.h>

#include <QPainterPath>
#include <QPixmap>

namespace car_track_ui {

class SceneUi : public physics_ui::Box2dSceneUi {
 public:
  SceneUi(car_track::Scene* scene);

  void render(QPainter& painter, const QRectF& viewport) override;
  void step() override;

 private:
  void renderCamera(QPainter& painter, const sim::Camera* camera) const;
  void renderDrone(QPainter& painter, const sim::Drone* drone) const;
  void renderPath(QPainter& painter) const;
  void renderTrack(QPainter& painter) const;
  void renderCurrentSegment(QPainter& painter) const;

 private:
  car_track::Scene* scene_ = nullptr;
  QPainterPath drone_path_;
  const QPixmap drone_pixmap_{ ":/resources/drone.png" };
};

}  // namespace car_track_ui
