
#pragma once

#include "tool_window.h"

#include <core_ui/sim/touch_widget.h>

class TouchWindow : public ToolWindow {
 public:
  explicit TouchWindow(QWidget* parent);

 private:
  void onSandboxChange(SandboxWindow* sandbox_window) override;

 private:
  physics_ui::TouchSensorWidget* sensor_widget_ = nullptr;
};
