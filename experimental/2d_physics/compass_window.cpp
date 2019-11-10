
#include "compass_window.h"
#include "sandbox_window.h"

#include <QVBoxLayout>

CompassWindow::CompassWindow(QWidget* parent) : ToolWindow(parent) {
  setWindowTitle("Compass");
  setMinimumSize(64, 64);
  sensor_widget_ = new physics_ui::CompassWidget(this);
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(sensor_widget_);
}

void CompassWindow::onSandboxChange(SandboxWindow* sandbox_window) {
  auto scene = sandbox_window ? sandbox_window->scenePackage().scene.get() : nullptr;
  sensor_widget_->setSensor(scene ? scene->compass() : nullptr);
}
