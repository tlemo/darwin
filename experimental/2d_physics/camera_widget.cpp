
#include "camera_widget.h"

#include <QPainter>

CameraWidget::CameraWidget(QWidget* parent) : QWidget(parent) {
  setMinimumSize(64, 64);
  setMaximumSize(4096, 64);
}

void CameraWidget::paintEvent(QPaintEvent*) {
  QPainter painter(this);
  painter.setPen(Qt::NoPen);
  painter.setBrush(Qt::green);
  painter.drawRect(rect());
}
