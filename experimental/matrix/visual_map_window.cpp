
#include "visual_map_window.h"

#include <QPainter>
#include <QVBoxLayout>

VisualMapWindow::VisualMapWindow(QWidget* parent) : ToolWindow(parent) {
  setWindowTitle("Visual Map");
  setMinimumSize(64, 64);
  visual_map_widget_ = new VisualMapWidget(this);
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(visual_map_widget_);
}

void VisualMapWindow::setScene(MapScene* scene) {
  visual_map_widget_->setImage(scene->world().visualMap());
}

void VisualMapWidget::paintEvent(QPaintEvent*) {
  if (image_) {
    QPainter painter(this);
    painter.drawImage(rect(), *image_);
  }
}
