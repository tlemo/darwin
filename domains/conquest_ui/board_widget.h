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
#include <domains/conquest/game.h>

#include <QColor>
#include <QPoint>
#include <QRect>

namespace conquest_ui {

class BoardWidget : public core_ui::Canvas {
  Q_OBJECT

  static constexpr int kBorderSize = 15;
  static constexpr int kNodeRadius = 30;
  static constexpr float kSx = 2.5f;
  static constexpr float kSy = 2.5f;

  const QColor kBlueColor{ 128, 128, 255 };
  const QColor kRedColor{ 255, 128, 128 };

 public:
  explicit BoardWidget(QWidget* parent);

  void setDebugRendering(bool debug);

  void reset();
  void setGame(conquest::Game* game);

 signals:
  void sigPause();
  void sigResume();

 protected:
  void paintEvent(QPaintEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;

 private:
  QRect nodeRect(int index) const;
  QPoint nodePos(int index) const;
  int hitTest(const QPoint& pos) const;
  void paintOrder(QPainter& painter,
                  const QColor& color,
                  int src,
                  const QPoint& dst_pos) const;
  void paintDeployment(QPainter& painter, int arc) const;
  void exitEditMode();
  void enterEditMode();
  int findArc(int src, int dst) const;

 private:
  conquest::Game* game_ = nullptr;

  bool debug_ = true;
  bool paused_ = false;

  int blue_order_ = -1;
  int red_order_ = -1;

  int highlight_ = -1;
  int src_ = -1;
  int dst_ = -1;

  QPoint mouse_pos_;  // tracked mouse pos, in viewport coord
};

}  // namespace conquest_ui
