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

#include <core_ui/canvas.h>
#include <third_party/box2d/box2d.h>

#include <QColor>

namespace core_ui {

class Box2dWidget : public core_ui::Canvas {
  Q_OBJECT

  const QColor kBackgroundColor{ 255, 255, 255 };
  const QColor kViewportColor{ 240, 240, 255 };

 public:
  explicit Box2dWidget(QWidget* parent);

  void setWorld(b2World* world, const QRectF& viewport);

 signals:
  void sigPlayPause();

 protected:
  void paintEvent(QPaintEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;

 private:
  b2World* world_ = nullptr;
};

}  // namespace core_ui
