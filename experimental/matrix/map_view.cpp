
#include "map_view.h"
#include "main_window.h"

#include <QWheelEvent>

MapView::MapView(QMainWindow* parent) : QGraphicsView(parent) {
  setFrameStyle(QFrame::Plain);

  setRenderHint(QPainter::Antialiasing, true);
  setDragMode(QGraphicsView::ScrollHandDrag);
  setOptimizationFlags(QGraphicsView::DontSavePainterState);
  setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
  setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
  setCacheMode(QGraphicsView::CacheNone);
}

void MapView::updateState(const vis::World& visible_state) {
  if (!scene_) {
    scene_ = new MapScene();
    setScene(scene_);
  }

  scene_->updateState(visible_state);
}

void MapView::wheelEvent(QWheelEvent* event) {
  if (event->angleDelta().y() > 0)
    emit zoomIn();
  else
    emit zoomOut();

  event->accept();
}

void MapView::paintEvent(QPaintEvent* event) {
  QGraphicsView::paintEvent(event);
  fps_tracker_.update();
}
