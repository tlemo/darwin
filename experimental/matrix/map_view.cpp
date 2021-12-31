
#include "map_view.h"
#include "main_window.h"

#include <QWheelEvent>
#include <QCursor>

MapView::MapView(QMainWindow* parent) : QGraphicsView(parent) {
  setFrameStyle(QFrame::Plain);

  setRenderHint(QPainter::Antialiasing, true);
  setDragMode(QGraphicsView::NoDrag);
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

void MapView::setMode(MapView::Mode mode) {
  mode_ = mode;
  switch (mode_) {
    case Mode::None:
      setDragMode(QGraphicsView::NoDrag);
      break;
    case Mode::Select:
      setDragMode(QGraphicsView::NoDrag);
      break;
    case Mode::Pan:
      setDragMode(QGraphicsView::ScrollHandDrag);
      break;
    default:
      FATAL("Unexpected mode");
  }
}

void MapView::wheelEvent(QWheelEvent* event) {
  if (event->angleDelta().y() > 0)
    emit zoomIn();
  else
    emit zoomOut();

  event->accept();
}

void MapView::paintEvent(QPaintEvent* event) {
  scene_->setCursorPosition(mapToScene(mapFromGlobal(QCursor::pos())),
                            mode_ == Mode::Select);
  QGraphicsView::paintEvent(event);
  fps_tracker_.update();
}
