
#include "touch_window.h"
#include "sandbox_window.h"

#include <QPainter>

TouchWindow::TouchWindow(QWidget* parent) : ToolWindow(parent) {
  setWindowTitle("Touch");
  setMinimumSize(64, 64);
  setContentsMargins(5, 5, 5, 5);
}

void TouchWindow::paintEvent(QPaintEvent*) {
  if (touch_sensor_ == nullptr) {
    return;
  }

  // TODO
}

void TouchWindow::onSandboxChange(SandboxWindow* sandbox_window) {
  // TODO
  update();
}
