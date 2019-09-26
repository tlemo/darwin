
#include "touch_window.h"
#include "sandbox_window.h"

#include <QPainter>

TouchWindow::TouchWindow(QWidget* parent) : ToolWindow(parent) {
  setWindowTitle("Touch");
  setMinimumSize(64, 64);
}

void TouchWindow::paintEvent(QPaintEvent* event) {
  ToolWindow::paintEvent(event);
  if (touch_sensor_ != nullptr) {
    // TODO
  }
}

void TouchWindow::onSandboxChange(SandboxWindow* sandbox_window) {
  // TODO
  update();
}
