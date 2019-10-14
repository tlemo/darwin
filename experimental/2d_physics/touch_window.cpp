
#include "touch_window.h"
#include "sandbox_window.h"

#include <QVBoxLayout>

TouchWindow::TouchWindow(QWidget* parent) : ToolWindow(parent) {
  setWindowTitle("Touch");
  setMinimumSize(64, 64);
  sensor_widget_ = new physics_ui::TouchSensorWidget(this);
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(sensor_widget_);
}

void TouchWindow::onSandboxChange(SandboxWindow* sandbox_window) {
  auto scene = sandbox_window ? sandbox_window->scenePackage().scene.get() : nullptr;
  sensor_widget_->setSensor(scene ? scene->touchSensor() : nullptr);
}
