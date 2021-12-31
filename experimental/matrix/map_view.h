
#pragma once

#include "map_scene.h"
#include "visible_world_state.h"

#include <core/rate_tracker.h>

#include <QMainWindow>
#include <QGraphicsView>

class MapView : public QGraphicsView {
  Q_OBJECT

 public:
  enum class Mode { None, Select, Pan };

 public:
  explicit MapView(QMainWindow* parent);

  void updateState(const vis::World& visible_state);

  void setMode(Mode mode);

  double fps() const { return fps_tracker_.currentRate(); }

 signals:
  void zoomIn();
  void zoomOut();

 private:
  void wheelEvent(QWheelEvent* event) override;
  void paintEvent(QPaintEvent* event) override;

 private:
  MapScene* scene_ = nullptr;
  core::RateTracker fps_tracker_;
  Mode mode_ = Mode::None;
};
