
#pragma once

#include "accelerometer.h"
#include "tool_window.h"

#include <core_ui/canvas.h>

class AccelerometerWidget : public core_ui::Canvas {
  static constexpr double kCanvasWidth = 20;
  static constexpr double kCanvasHeight = 20;

  static constexpr double kSensorWidth = 15;
  static constexpr double kSensorHeight = 15;

  static constexpr double kSkinSize = 0.7;
  static constexpr double kVectorLength = 6;
  static constexpr double kVectorWidth = 0.4;

 public:
  explicit AccelerometerWidget(QWidget* parent);

  void setSensor(const phys::Accelerometer* sensor);

 private:
  void paintEvent(QPaintEvent* event) override;

 private:
  const phys::Accelerometer* sensor_ = nullptr;
};

class AccelerometerWindow : public ToolWindow {
 public:
  explicit AccelerometerWindow(QWidget* parent);

 private:
  void onSandboxChange(SandboxWindow* sandbox_window) override;

 private:
  AccelerometerWidget* sensor_widget_ = nullptr;
};
