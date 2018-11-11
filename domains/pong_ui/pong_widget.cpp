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

#include "pong_widget.h"

#include <QBrush>
#include <QKeyEvent>
#include <QPainter>
#include <QPen>
#include <QRect>
#include <QRectF>

#include <assert.h>
#include <math.h>
#include <limits>
using namespace std;

namespace pong_ui {

void PongWidget::gameStep() {
  if (!game_->gameStep())
    game_->newSet();

  update();

  emit sigUpdate();
}

PongWidget::PongWidget(QWidget* parent) : QFrame(parent) {
  setAutoFillBackground(false);
  setMinimumSize(100, 50);
  setFocusPolicy(Qt::StrongFocus);
}

void PongWidget::reset() {
  game_ = nullptr;
  timer_.stop();
  timer_.disconnect();
}

void PongWidget::setGame(pong::Game* game) {
  Q_ASSERT(game != nullptr);
  Q_ASSERT(game_ == nullptr);

  game_ = game;
  timer_.stop();
  timer_.disconnect();
  connect(&timer_, &QTimer::timeout, this, &PongWidget::gameStep);
  timer_.start(33);
  paused_ = false;
  update();
}

void PongWidget::setDebugRendering(bool debug) {
  debug_ = debug;
  update();
}

void PongWidget::paintPaddle(QPainter& painter, float x, float y) const {
  const float height = pong::g_config.paddle_size;

  QRectF paddle_rect(x < 0 ? x - kPaddleWidth : x, y - height / 2, kPaddleWidth, height);

  painter.setPen(Qt::NoPen);
  painter.setBrush(kPaddleColor);
  painter.drawRect(paddle_rect);
}

void PongWidget::paintTrajectory(QPainter& painter,
                                 const pong::Game::Ball& ball,
                                 int bounce_count) const {
  // limit the max number of traced "bounces"
  if (bounce_count > 10)
    return;

  painter.setPen(QPen(kDebugLineColor, 0, Qt::DotLine, Qt::SquareCap, Qt::MiterJoin));

  const float offset = pong::g_config.paddle_offset;
  const float r = pong::g_config.ball_radius;
  const float left = -1 + offset + r;
  const float right = 1 - offset - r;
  const float up = 1 - r;
  const float down = r;

  float t = numeric_limits<float>::infinity();
  bool bounce = false;

  Q_ASSERT(ball.vx != 0);
  float tx_left = (left - ball.x) / ball.vx;
  float tx_right = (right - ball.x) / ball.vx;

  if (tx_left > 0 && tx_left < t)
    t = tx_left;
  else if (tx_right > 0 && tx_right < t)
    t = tx_right;

  if (ball.vy > 0) {
    float tx_up = (up - ball.y) / ball.vy;
    if (tx_up > 0 && tx_up < t) {
      t = tx_up;
      bounce = true;
    }
  } else if (ball.vy < 0) {
    float tx_down = (down - ball.y) / ball.vy;
    if (tx_down > 0 && tx_down < t) {
      t = tx_down;
      bounce = true;
    }
  }

  float xt = ball.x + ball.vx * t;
  float yt = ball.y + ball.vy * t;

  painter.drawLine(QLineF(ball.x, ball.y, xt, yt));

  if (bounce) {
    auto bounced_ball = ball;
    bounced_ball.x = xt;
    bounced_ball.y = yt;
    bounced_ball.vy = -bounced_ball.vy;
    paintTrajectory(painter, bounced_ball, bounce_count + 1);
  }
}

void PongWidget::paintBoard(QPainter& painter) const {
  bool active = game_ != nullptr && hasFocus();
  const QColor edge_color = active ? kBoardEdgeColor : kDisabledEdgeColor;

  // board outline
  const float hw = kBoardEdgeWidth / 2;
  painter.setPen(
      QPen(edge_color, kBoardEdgeWidth, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
  painter.setBrush(kBoardBackgroundColor);
  painter.drawRect(QRectF(-1 - hw, -hw, 2.0 + 2 * hw, 1.0 + 2 * hw));

  // "net"
  painter.setPen(
      QPen(edge_color, kBoardEdgeWidth, Qt::DotLine, Qt::SquareCap, Qt::MiterJoin));
  painter.drawLine(QLineF(0, 0, 0, 1));

  if (debug_) {
    const float dy = pong::g_config.ball_radius;
    const float dx = pong::g_config.paddle_offset + dy;
    painter.setPen(QPen(kDebugLineColor, 0, Qt::DotLine, Qt::SquareCap, Qt::MiterJoin));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(QRectF(QPointF(-1 + dx, dy), QPointF(1 - dx, 1 - dy)));
  }
}

void PongWidget::paintScore(QPainter& painter, float x, float y, int score) const {
  constexpr float kScale = 1 / 100.0f;

  painter.save();
  painter.scale(kScale, -kScale);

  auto text = QString::asprintf("%d", score);
  auto rect = QRectF(x / kScale - 20, -y / kScale - 20, 40, 40);

  painter.setFont(QFont("Arial", 10, QFont::Bold));
  painter.setPen(kScoreColor);
  painter.drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, text);

  painter.restore();
}

void PongWidget::paintMessage(QPainter& painter,
                              float x,
                              float y,
                              const QString& message) const {
  constexpr float kScale = 1 / 100.0f;

  painter.save();
  painter.scale(kScale, -kScale);

  constexpr int kWidth = 1000;
  constexpr int kHeight = 500;

  auto rect = QRectF(x / kScale - kWidth / 2, -y / kScale - kHeight / 2, kWidth, kHeight);

  painter.setFont(QFont("Arial", 12, QFont::Bold));
  painter.setPen(kMessageColor);
  painter.drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, message);

  painter.restore();
}

void PongWidget::paintBall(QPainter& painter, const pong::Game::Ball& ball) const {
  const float r = pong::g_config.ball_radius;

  if (debug_)
    paintTrajectory(painter, ball);

  painter.setPen(Qt::NoPen);
  painter.setBrush(kBallColor);
  painter.drawEllipse(QPointF(ball.x, ball.y), r, r);
}

void PongWidget::paintEvent(QPaintEvent*) {
  QPainter painter(this);

  painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

  // background
  painter.setPen(Qt::NoPen);
  painter.setBrush(kBackgroundColor);
  painter.drawRect(rect());

  // calculate board rect
  QRect board_rect = rect();
  board_rect.adjust(kBorderSize, kBorderSize, -kBorderSize, -kBorderSize);

  // adjust the width or the height to keep the target ratio
  constexpr float target_ratio = 2.0f;  // width / height
  float scale = 0;
  if (float(board_rect.width()) / board_rect.height() > target_ratio) {
    board_rect.setWidth(board_rect.height() * target_ratio);
    scale = board_rect.height() / 1.0f;
  } else {
    board_rect.setHeight(board_rect.width() / target_ratio);
    scale = board_rect.width() / 2.0f;
  }
  board_rect.moveCenter(rect().center());

  painter.translate(board_rect.center().x(), board_rect.bottom());
  painter.scale(scale, -scale);

  paintBoard(painter);

  if (game_ == nullptr)
    return;

  // scores
  paintScore(painter, -0.5f, 0.8f, game_->scoreP1());
  paintScore(painter, 0.5f, 0.8f, game_->scoreP2());

  // paddles
  const float offset = pong::g_config.paddle_offset;
  paintPaddle(painter, -1 + offset, game_->paddlePosP1());
  paintPaddle(painter, 1 - offset, game_->paddlePosP2());

  paintBall(painter, game_->ball());

  if (paused_)
    paintMessage(painter, 0, 0.5f, "PAUSED");
}

void PongWidget::keyPressEvent(QKeyEvent* event) {
  key_state_[event->key()] = true;
}

void PongWidget::keyReleaseEvent(QKeyEvent* event) {
  key_state_[event->key()] = false;
}

void PongWidget::focusInEvent(QFocusEvent*) {
  update();
}

void PongWidget::focusOutEvent(QFocusEvent*) {
  key_state_.clear();
  update();
}

void PongWidget::mousePressEvent(QMouseEvent*) {
  paused_ = !paused_;
  if (paused_)
    timer_.stop();
  else if (game_ != nullptr)
    timer_.start();
  update();
}

}  // namespace pong_ui
