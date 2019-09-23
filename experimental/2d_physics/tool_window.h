
#pragma once

#include <QFrame>

class SandboxWindow;

class ToolWindow : public QFrame {
  Q_OBJECT

 public:
  explicit ToolWindow(QWidget* parent) : QFrame(parent) {}

  virtual void onSandboxChange(SandboxWindow* sandbox_window) = 0;
};
