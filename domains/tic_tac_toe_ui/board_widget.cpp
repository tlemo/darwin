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
#include "human_player.h"

#include <QBrush>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPointF>
#include <QRectF>

namespace tic_tac_toe_ui {

BoardWidget::BoardWidget(QWidget* parent) : core_ui::Canvas(parent) {
  setAutoFillBackground(false);
  setBorderSize(kBorderSize);
  setViewport(QRectF(0, 0, 3, 3));
}

void BoardWidget::setGame(Game* game) {
  Q_ASSERT(game != nullptr);
  Q_ASSERT(game_ == nullptr);

  game_ = game;
  update();
}

void BoardWidget::paintEvent(QPaintEvent*) {
  QPainter painter(this);

  painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

  // background
  painter.setPen(Qt::NoPen);
  painter.setBrush(Qt::white);
  painter.drawRect(rect());

  painter.setTransform(transformFromViewport());

  // board's grid
  painter.setPen(QPen(kGridColor, 0.05));
  painter.setBrush(Qt::NoBrush);
  for (int i = 1; i < 3; ++i) {
    painter.drawLine(0, i, 3, i);
    painter.drawLine(i, 0, i, 3);
  }

  // board configuration
  if (game_ != nullptr) {
    painter.setBrush(Qt::NoBrush);

    const auto& board = game_->board();

    // highlight the winning line, if any
    for (const auto& line : tic_tac_toe::Board::kLines) {
      auto c0 = board[line[0]];
      auto c1 = board[line[1]];
      auto c2 = board[line[2]];
      if (c0 != tic_tac_toe::Board::Piece::Empty && c0 == c1 && c1 == c2) {
        auto squareIndexToPoint = [](int square) {
          CHECK(square >= 0 && square < 9);
          int col = square % 3;
          int row = square / 3;
          return QPointF(col + 0.5, row + 0.5);
        };

        QPainterPath path;
        path.moveTo(squareIndexToPoint(line[0]));
        path.lineTo(squareIndexToPoint(line[1]));
        path.lineTo(squareIndexToPoint(line[2]));
        painter.setPen(QPen(kWinningLineColor, 0.2));
        painter.drawPath(path);
        break;
      }
    }

    // render the pieces
    const int last_move = game_->lastMove();
    for (int row = 0; row < 3; ++row) {
      for (int col = 0; col < 3; ++col) {
        const int square_index = row * 3 + col;

        const auto piece_color =
            (square_index == last_move) ? kLastPieceColor : kPieceColor;
        painter.setPen(QPen(piece_color, 0.05));

        QRectF piece_rect(col, row, 1, 1);
        piece_rect.adjust(0.25, 0.25, -0.25, -0.25);

        switch (board[square_index]) {
          case tic_tac_toe::Board::Piece::X:
            painter.drawLine(piece_rect.topLeft(), piece_rect.bottomRight());
            painter.drawLine(piece_rect.bottomLeft(), piece_rect.topRight());
            break;

          case tic_tac_toe::Board::Piece::Zero:
            painter.drawEllipse(piece_rect);
            break;

          default:
            break;
        }
      }
    }
  }
}

void BoardWidget::mousePressEvent(QMouseEvent* event) {
  selected_square_ = hitTest(transformToViewport().map(event->localPos()));
}

void BoardWidget::mouseReleaseEvent(QMouseEvent* event) {
  const int square = hitTest(transformToViewport().map(event->localPos()));
  if (square != tic_tac_toe::Board::kNoSquare && square == selected_square_) {
    auto human_player = dynamic_cast<HumanPlayer*>(game_->currentPlayer());
    if (human_player != nullptr) {
      human_player->selectMove(square);
      emit sigReady();
    }
  }
  selected_square_ = tic_tac_toe::Board::kNoSquare;
}

void BoardWidget::leaveEvent(QEvent*) {
  selected_square_ = tic_tac_toe::Board::kNoSquare;
  update();
}

int BoardWidget::hitTest(const QPointF& pos) const {
  const int col = int(pos.x());
  const int row = int(pos.y());

  return (col >= 0 && col < 3 && row >= 0 && row < 3) ? row * 3 + col
                                                      : tic_tac_toe::Board::kNoSquare;
}

}  // namespace tic_tac_toe_ui
