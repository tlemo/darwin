
#pragma once

#include "tool_window.h"
#include "touch_sensor.h"

class TouchWindow : public ToolWindow {
 public:
  explicit TouchWindow(QWidget* parent);

 private:
  void paintEvent(QPaintEvent* event) override;
  void onSandboxChange(SandboxWindow* sandbox_window) override;

 private:
  const phys::TouchSensor* touch_sensor_ = nullptr;
};
