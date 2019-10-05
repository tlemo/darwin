
#pragma once

#include "camera.h"
#include "tool_window.h"

class CameraWindow : public ToolWindow {
 public:
  explicit CameraWindow(QWidget* parent, const phys::Camera* camera = nullptr);

 private:
  void paintEvent(QPaintEvent* event) override;
  void onSandboxChange(SandboxWindow* sandbox_window) override;

 private:
  const phys::Camera* camera_ = nullptr;
};
