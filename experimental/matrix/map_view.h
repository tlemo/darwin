
#pragma once

#include <QMainWindow>
#include <QGraphicsView>

class MapView : public QGraphicsView {
  Q_OBJECT

 public:
  explicit MapView(QMainWindow* parent);

 signals:
  void zoomIn();
  void zoomOut();

 protected:
  void wheelEvent(QWheelEvent* e) override;
  void paintEvent(QPaintEvent* event) override;
};
