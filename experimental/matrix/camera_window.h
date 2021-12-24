
#pragma once

#include "tool_window.h"

#include <core_ui/sim/camera_widget.h>

class CameraWindow : public ToolWindow {
 public:
  explicit CameraWindow(QWidget* parent);

 private:
  physics_ui::CameraWidget* camera_widget_ = nullptr;
};
