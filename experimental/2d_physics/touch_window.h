
#pragma once

#include "tool_window.h"
#include "touch_sensor.h"

#include <core_ui/canvas.h>

class TouchSensorWidget : public core_ui::Canvas {
 public:
  explicit TouchSensorWidget(QWidget* parent);

  void setSensor(const phys::TouchSensor* sensor);

 private:
  void paintEvent(QPaintEvent* event) override;

 private:
  const phys::TouchSensor* sensor_ = nullptr;
};

class TouchWindow : public ToolWindow {
 public:
  explicit TouchWindow(QWidget* parent);

 private:
  void onSandboxChange(SandboxWindow* sandbox_window) override;

 private:
  TouchSensorWidget* touch_sensor_widget_ = nullptr;
};
