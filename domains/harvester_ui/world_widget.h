// Copyright 2018 The Darwin Neuroevolution Framework Authors.
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
#include <domains/harvester/world.h>

#include <QColor>

namespace harvester_ui {

class WorldWidget : public core_ui::Canvas {
  Q_OBJECT

  const QColor kBackgroundColor{ 255, 255, 255 };
  const QColor kEmptyColor{ 240, 240, 240 };
  const QColor kGridColor{ 160, 160, 160 };
  const QColor kWallColor{ 128, 128, 128 };
  const QColor kBadFruitColor{ 255, 0, 0 };
  const QColor kJunkFruitColor{ 240, 240, 0 };
  const QColor kGoodFruitColor{ 0, 255, 0 };
  const QColor kVisitedColor{ 220, 220, 220 };
  const QColor kRobotColor{ 64, 64, 255 };
  const QColor kFovColor{ 0, 0, 255, 16 };

 public:
  explicit WorldWidget(QWidget* parent);

  void setWorld(const harvester::World* world);

 signals:
  void sigSingleStep();

 protected:
  void paintEvent(QPaintEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;

 private:
  void paintWorld(QPainter& painter) const;
  void paintRobot(QPainter& painter) const;

 private:
  const harvester::World* world_ = nullptr;
  int rows_ = 0;
  int cols_ = 0;
};

}  // namespace harvester_ui
