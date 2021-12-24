
#include "touch_window.h"

#include <QVBoxLayout>

TouchWindow::TouchWindow(QWidget* parent) : ToolWindow(parent) {
  setWindowTitle("Touch");
  setMinimumSize(64, 64);
  sensor_widget_ = new physics_ui::TouchSensorWidget(this);
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(sensor_widget_);
}

