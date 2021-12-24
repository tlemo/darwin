
#pragma once

#include "tool_window.h"

#include <core_ui/sim/accelerometer_widget.h>

class AccelerometerWindow : public ToolWindow {
 public:
  explicit AccelerometerWindow(QWidget* parent);

 private:
  physics_ui::AccelerometerWidget* sensor_widget_ = nullptr;
};
