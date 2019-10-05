
#pragma once

#include "compass.h"
#include "tool_window.h"

#include <core_ui/canvas.h>

class CompassWidget : public core_ui::Canvas {
  static constexpr double kCanvasWidth = 20;
  static constexpr double kCanvasHeight = 20;

  static constexpr double kSensorWidth = 15;
  static constexpr double kSensorHeight = 15;

  static constexpr double kCompassLength = 7;
  static constexpr double kCompassWidth = 0.4;

 public:
  explicit CompassWidget(QWidget* parent);

  void setSensor(const phys::Compass* sensor);

 private:
  void paintEvent(QPaintEvent* event) override;

 private:
  const phys::Compass* sensor_ = nullptr;
};

class CompassWindow : public ToolWindow {
 public:
  explicit CompassWindow(QWidget* parent);

 private:
  void onSandboxChange(SandboxWindow* sandbox_window) override;

 private:
  CompassWidget* sensor_widget_ = nullptr;
};
