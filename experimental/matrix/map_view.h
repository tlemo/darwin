
#pragma once

#include "map_scene.h"

#include <core/rate_tracker.h>

#include <QMainWindow>
#include <QGraphicsView>

class MapView : public QGraphicsView {
  Q_OBJECT

 public:
  enum class Mode { None, Select, Pan };

 public:
  explicit MapView(QMainWindow* parent);

  void refresh();

  void setMode(Mode mode);

  auto mapScene() { return &scene_; }

  double timestamp() const { return scene_.world().timestamp(); }
  double ups() const { return scene_.world().ups(); }
  double fps() const { return fps_tracker_.currentRate(); }

 signals:
  void zoomIn();
  void zoomOut();

 private:
  void wheelEvent(QWheelEvent* event) override;
  void paintEvent(QPaintEvent* event) override;

 private:
  MapScene scene_;
  core::RateTracker fps_tracker_;
  Mode mode_ = Mode::None;
};
