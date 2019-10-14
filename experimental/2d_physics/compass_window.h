
#pragma once

#include "tool_window.h"

#include <core_ui/physics/compass_widget.h>

class CompassWindow : public ToolWindow {
 public:
  explicit CompassWindow(QWidget* parent);

 private:
  void onSandboxChange(SandboxWindow* sandbox_window) override;

 private:
  physics_ui::CompassWidget* sensor_widget_ = nullptr;
};
