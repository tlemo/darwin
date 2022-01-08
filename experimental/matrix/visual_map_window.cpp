
#include "visual_map_window.h"

#include <QVBoxLayout>

VisualMapWindow::VisualMapWindow(QWidget* parent) : ToolWindow(parent) {
  setWindowTitle("Visual Map");
  setMinimumSize(64, 64);
  visual_map_widget_ = new VisualMapWidget(this);
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(visual_map_widget_);
}
