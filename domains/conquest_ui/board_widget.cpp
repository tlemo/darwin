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

#include "board_widget.h"

#include <core/utils.h>
#include <domains/conquest/board.h>
#include <domains/conquest/game.h>

#include <QBrush>
#include <QMargins>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QRect>
#include <QRectF>
#include <QRegion>
#include <QString>

#include <math.h>
using namespace std;

namespace conquest_ui {

BoardWidget::BoardWidget(QWidget* parent) : core_ui::Canvas(parent) {
  setAutoFillBackground(false);

  setBorderSize(kBorderSize);
  setViewport(QPointF(-300, -300), QPointF(300, 300));
}

void BoardWidget::setDebugRendering(bool debug) {
  debug_ = debug;
  update();
}

void BoardWidget::reset() {
  game_ = nullptr;
}

void BoardWidget::setGame(conquest::Game* game) {
  Q_ASSERT(game != nullptr);
  Q_ASSERT(game_ == nullptr);

  game_ = game;
  exitEditMode();
  update();
}

void BoardWidget::enterEditMode() {
  Q_ASSERT(blue_order_ == -1);
  Q_ASSERT(red_order_ == -1);
  Q_ASSERT(src_ == -1);
  Q_ASSERT(dst_ == -1);
  Q_ASSERT(highlight_ == -1);

  paused_ = true;
  setMouseTracking(true);
}

void BoardWidget::exitEditMode() {
  // apply orders
  if (blue_order_ != -1)
    game_->order(blue_order_);
  if (red_order_ != -1)
    game_->order(red_order_);

  // reset state
  blue_order_ = -1;
  red_order_ = -1;
  src_ = -1;
  dst_ = -1;
  highlight_ = -1;

  paused_ = false;
  setMouseTracking(false);
}

void BoardWidget::mousePressEvent(QMouseEvent* event) {
  mouse_pos_ = transformToViewport().map(event->pos());
  int pick = hitTest(mouse_pos_);

  if (paused_) {
    // exit edit mode?
    if (pick == -1) {
      exitEditMode();
      emit sigResume();
    }
  } else {
    enterEditMode();
    emit sigPause();
  }

  if (paused_ && pick != -1) {
    Q_ASSERT(src_ == -1);
    Q_ASSERT(dst_ == -1);

    float units = game_->nodeUnits(pick);
    if (units != 0) {
      src_ = pick;

      if (units > 0)
        blue_order_ = -1;
      else
        red_order_ = -1;
    }
  }

  update();
}

int BoardWidget::findArc(int src, int dst) const {
  const auto& arcs = game_->board()->arcs;
  for (int i = 0; i < arcs.size(); ++i) {
    if (arcs[i].src == src && arcs[i].dst == dst)
      return i;
  }

  return -1;
}

void BoardWidget::mouseReleaseEvent(QMouseEvent* event) {
  mouse_pos_ = transformToViewport().map(event->pos());

  // complete order?
  if (dst_ != -1) {
    Q_ASSERT(src_ != -1);
    float units = game_->nodeUnits(src_);
    int& order = (units > 0) ? blue_order_ : red_order_;
    order = findArc(src_, dst_);
    Q_ASSERT(order != -1);
  }

  src_ = -1;
  dst_ = -1;
  update();
}

void BoardWidget::mouseMoveEvent(QMouseEvent* event) {
  if (!paused_)
    return;

  mouse_pos_ = transformToViewport().map(event->pos());

  int pick = hitTest(mouse_pos_);

  // update highlighted node?
  if (pick != highlight_) {
    highlight_ = pick;
  }

  // update dst
  dst_ = -1;
  if (src_ != -1 && pick != -1 && findArc(src_, pick) != -1) {
    dst_ = pick;
  }

  update();
}

QRect BoardWidget::nodeRect(int index) const {
  QRect rect(-kNodeRadius, -kNodeRadius, kNodeRadius * 2, kNodeRadius * 2);
  rect.translate(nodePos(index));
  return rect;
}

QPoint BoardWidget::nodePos(int index) const {
  const auto& node = game_->board()->nodes[index];
  return QPoint(node.x * kSx, node.y * kSy);
}

// pos is in logical coordinates
int BoardWidget::hitTest(const QPoint& pos) const {
  for (int i = 0; i < game_->board()->nodes.size(); ++i) {
    QRect rect = nodeRect(i);
    rect += QMargins(10, 10, 10, 10);

    QRegion hit_region(rect, QRegion::Ellipse);
    if (hit_region.contains(pos)) {
      return i;
    }
  }

  return -1;
}

void BoardWidget::paintOrder(QPainter& painter,
                             const QColor& color,
                             int src,
                             const QPoint& dst_pos) const {
  QRect src_rect = nodeRect(src);
  src_rect += QMargins(5, 5, 5, 5);

  painter.setPen(QPen(color, 2, Qt::DashLine));
  painter.setBrush(Qt::NoBrush);
  painter.drawEllipse(src_rect);

  QPoint src_pos = nodePos(src);

  int dx = dst_pos.x() - src_pos.x();
  int dy = dst_pos.y() - src_pos.y();

  double len = sqrt(dx * dx + dy * dy);

  if (len > 1) {
    dx = int(dx / len * kNodeRadius);
    dy = int(dy / len * kNodeRadius);

    QPoint p1 = src_pos + QPoint(dy, -dx);
    QPoint p2 = src_pos + QPoint(-dy, dx);

    painter.drawLine(p1, dst_pos);
    painter.drawLine(p2, dst_pos);
  }
}

void BoardWidget::paintDeployment(QPainter& painter, int arc_index) const {
  const auto& arc = game_->board()->arcs[arc_index];

  float size = game_->deployment(arc_index).size;
  float pos = game_->deployment(arc_index).position;
  Q_ASSERT(pos >= 0 && pos <= 1.0f);

  if (size == 0)
    return;

  QColor color = size > 0 ? kBlueColor : kRedColor;

  QPoint A = nodePos(arc.src);
  QPoint B = nodePos(arc.dst);

  int dx = B.x() - A.x();
  int dy = B.y() - A.y();
  Q_ASSERT(dx != 0 || dy != 0);

  double len = sqrt(dx * dx + dy * dy);
  double a = (kNodeRadius + (len - 2 * kNodeRadius) * pos) / len;

  QPoint D = A + QPoint(dx * a, dy * a);
  QPoint E = D + QPoint(dy / len * kNodeRadius, -dx / len * kNodeRadius);

  painter.setPen(QPen(color, 2));
  painter.setBrush(color);
  painter.drawLine(D, E);
  painter.drawEllipse(D, 3, 3);

  QRect rect(E.x() - 20, E.y() - 8, 40, 16);
  painter.setPen(QPen(Qt::black, 0.5));
  painter.drawRect(rect);

  QString text;
  if (debug_)
    text = QString::asprintf("%.2f", size);
  else
    text = QString::asprintf("%d", int(fabsf(size) * conquest::g_config.int_unit_scale));
  painter.setFont(QFont("Arial", 8));
  painter.drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, text);
}

void BoardWidget::paintEvent(QPaintEvent*) {
  const conquest::Board* board = game_->board();

  QPainter painter(this);

  painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

  // background
  painter.setPen(Qt::NoPen);
  painter.setBrush(paused_ ? QColor(230, 240, 240) : Qt::white);
  painter.drawRect(rect());

  painter.setTransform(transformFromViewport());

  // draw edges
  painter.setPen(QPen(Qt::black, 1.5, paused_ ? Qt::DashLine : Qt::SolidLine));
  painter.setBrush(Qt::NoBrush);
  for (int i = 0; i < board->arcs.size(); ++i) {
    const auto& arc = board->arcs[i];
    if (arc.src < arc.dst) {
      painter.drawLine(nodePos(arc.src), nodePos(arc.dst));
    }
  }

  // selection?
  if (highlight_ != -1) {
    QRect rect = nodeRect(highlight_);
    rect += QMargins(5, 5, 5, 5);

    painter.setPen(QPen(Qt::lightGray, 3, Qt::DashLine));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(rect);
  }

  // tentative order?
  if (src_ != -1) {
    QColor color(Qt::lightGray);
    QPoint dst_pos = mouse_pos_;

    if (dst_ != -1) {
      float units = game_->nodeUnits(src_);
      color = (units > 0) ? kBlueColor : kRedColor;
      dst_pos = nodePos(dst_);
    }

    paintOrder(painter, color, src_, dst_pos);
  }

  // blue order?
  if (blue_order_ != -1) {
    const auto& arc = board->arcs[blue_order_];
    paintOrder(painter, kBlueColor, arc.src, nodePos(arc.dst));
  }

  // red order?
  if (red_order_ != -1) {
    const auto& arc = board->arcs[red_order_];
    paintOrder(painter, kRedColor, arc.src, nodePos(arc.dst));
  }

  // draw nodes
  painter.setPen(QPen(Qt::black, 2));
  if (debug_)
    painter.setFont(QFont("Arial", 10, QFont::Normal));
  else
    painter.setFont(QFont("Arial", 14, QFont::Bold));

  for (int i = 0; i < board->nodes.size(); ++i) {
    float units = game_->nodeUnits(i);
    QRect rect = nodeRect(i);

    if (units == 0)
      painter.setBrush(Qt::lightGray);
    else if (units > 0)
      painter.setBrush(kBlueColor);
    else
      painter.setBrush(kRedColor);

    painter.drawEllipse(rect);

    if (game_->state() == conquest::Game::State::None)
      continue;

    if (debug_) {
      auto text = QString::asprintf("Node%d\n%.2f", i, units);
      painter.drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, text);
    } else {
      auto text =
          QString::asprintf("%d", int(fabsf(units) * conquest::g_config.int_unit_scale));
      painter.drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, text);
    }
  }

  // draw deployments
  for (int i = 0; i < board->arcs.size(); ++i) {
    paintDeployment(painter, i);
  }
}

}  // namespace conquest_ui
