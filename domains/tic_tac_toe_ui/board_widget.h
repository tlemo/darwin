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

#include "game.h"

#include <core_ui/canvas.h>

#include <QColor>
#include <QPointF>

namespace tic_tac_toe_ui {

class BoardWidget : public core_ui::Canvas {
  Q_OBJECT

  static constexpr int kBorderSize = 15;

  const QColor kGridColor{ 128, 128, 128 };
  const QColor kPieceColor{ 32, 32, 32 };
  const QColor kLastPieceColor{ 32, 32, 200 };
  const QColor kWinningLineColor{ 64, 128, 64, 64 };

 public:
  explicit BoardWidget(QWidget* parent);

  void setGame(Game* game);

 signals:
  void sigReady();

 protected:
  void paintEvent(QPaintEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void leaveEvent(QEvent* event) override;

 private:
  int hitTest(const QPointF& pos) const;

 private:
  Game* game_ = nullptr;
  int selected_square_ = tic_tac_toe::Board::kNoSquare;
};

}  // namespace tic_tac_toe_ui
