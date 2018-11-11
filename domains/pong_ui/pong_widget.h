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

#include <domains/pong/game.h>

#include <QColor>
#include <QFrame>
#include <QTimer>

#include <unordered_map>
using namespace std;

namespace pong_ui {

// TODO: use core_ui::Canvas
class PongWidget : public QFrame {
  Q_OBJECT

  static constexpr int kBorderSize = 15;
  static constexpr float kBoardEdgeWidth = 0.01f;
  static constexpr float kPaddleWidth = 0.015f;

  const QColor kBackgroundColor{ 255, 255, 255 };
  const QColor kBoardBackgroundColor{ 240, 240, 255 };
  const QColor kBoardEdgeColor{ 64, 64, 128 };
  const QColor kPaddleColor{ 64, 128, 64 };
  const QColor kBallColor{ 216, 64, 64 };
  const QColor kDisabledEdgeColor{ 128, 128, 128 };
  const QColor kDebugLineColor{ 128, 128, 128 };
  const QColor kScoreColor{ 128, 128, 128 };
  const QColor kMessageColor{ 128, 200, 128, 128 };

 public:
  explicit PongWidget(QWidget* parent);

  void reset();
  void setGame(pong::Game* game);

  int keyState(int key) const {
    auto it = key_state_.find(key);
    return it != key_state_.end() ? it->second : false;
  }

  void setDebugRendering(bool debug);

 signals:
  void sigUpdate();

 private:
  void paintEvent(QPaintEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;
  void keyReleaseEvent(QKeyEvent* event) override;
  void focusInEvent(QFocusEvent*) override;
  void focusOutEvent(QFocusEvent*) override;
  void mousePressEvent(QMouseEvent*) override;

  void paintPaddle(QPainter& painter, float x, float y) const;
  void paintBall(QPainter& painter, const pong::Game::Ball& ball) const;
  void paintTrajectory(QPainter& painter,
                       const pong::Game::Ball& ball,
                       int bounce_count = 0) const;
  void paintBoard(QPainter& painter) const;
  void paintScore(QPainter& painter, float x, float y, int score) const;
  void paintMessage(QPainter& painter, float x, float y, const QString& message) const;

  void gameStep();

 private:
  pong::Game* game_ = nullptr;
  unordered_map<int, bool> key_state_;
  bool debug_ = true;
  bool paused_ = true;

  QTimer timer_;
};

}  // namespace pong_ui
