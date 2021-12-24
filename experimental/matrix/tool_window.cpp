
#include "tool_window.h"

ToolWindow::ToolWindow(QWidget* parent) : QFrame(parent) {
  setContentsMargins(5, 5, 5, 5);
  setStyleSheet(R"(
    ToolWindow
    {
      border: 1px solid lightgray;
      border-radius: 2px;
    })");
}
