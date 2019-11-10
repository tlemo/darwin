
#pragma once

#include "tool_window.h"

#include <core_ui/sim/accelerometer_widget.h>

class AccelerometerWindow : public ToolWindow {
 public:
  explicit AccelerometerWindow(QWidget* parent);

 private:
  void onSandboxChange(SandboxWindow* sandbox_window) override;

 private:
  physics_ui::AccelerometerWidget* sensor_widget_ = nullptr;
};
