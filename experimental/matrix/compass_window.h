
#pragma once

#include "tool_window.h"

#include <core_ui/sim/compass_widget.h>

class CompassWindow : public ToolWindow {
 public:
  explicit CompassWindow(QWidget* parent);

 private:
  physics_ui::CompassWidget* sensor_widget_ = nullptr;
};
