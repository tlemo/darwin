
#pragma once

#include "tool_window.h"

#include <core_ui/physics/camera_widget.h>

class CameraWindow : public ToolWindow {
 public:
  explicit CameraWindow(QWidget* parent);

 private:
  void onSandboxChange(SandboxWindow* sandbox_window) override;

 private:
  physics_ui::CameraWidget* camera_widget_ = nullptr;
};
