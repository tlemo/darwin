
#pragma once

#include "tool_window.h"
#include "map_scene.h"

#include <QImage>
#include <QWidget>

class VisualMapWidget : public QWidget {
 public:
  explicit VisualMapWidget(QWidget* parent) : QWidget(parent) {}

  void setImage(const QImage* image) { image_ = image; }

 private:
  void paintEvent(QPaintEvent* event) override;

 private:
  const QImage* image_ = nullptr;
};

class VisualMapWindow : public ToolWindow {
 public:
  explicit VisualMapWindow(QWidget* parent);

  void setScene(MapScene* scene);

 private:
  VisualMapWidget* visual_map_widget_ = nullptr;
};
