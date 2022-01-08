
#pragma once

#include "tool_window.h"

#include <QImage>
#include <QWidget>

class VisualMapWidget : public QWidget {
 public:
  explicit VisualMapWidget(QWidget* parent) : QWidget(parent) {}

 private:
  QImage* image_ = nullptr;
};

class VisualMapWindow : public ToolWindow {
 public:
  explicit VisualMapWindow(QWidget* parent);

 private:
  VisualMapWidget* visual_map_widget_ = nullptr;
};
