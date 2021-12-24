
#include "compass_window.h"

#include <QVBoxLayout>

CompassWindow::CompassWindow(QWidget* parent) : ToolWindow(parent) {
  setWindowTitle("Compass");
  setMinimumSize(64, 64);
  sensor_widget_ = new physics_ui::CompassWidget(this);
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(sensor_widget_);
}

