
#include "accelerometer_window.h"

#include <QVBoxLayout>

AccelerometerWindow::AccelerometerWindow(QWidget* parent) : ToolWindow(parent) {
  setWindowTitle("Accelerometer");
  setMinimumSize(64, 64);
  sensor_widget_ = new physics_ui::AccelerometerWidget(this);
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(sensor_widget_);
}

