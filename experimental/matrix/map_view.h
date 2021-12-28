
#pragma once

#include "main_window.h"

#include <QGraphicsView>

class MapView : public QGraphicsView {
  Q_OBJECT

 public:
  explicit MapView(MainWindow* parent);

 signals:
  void zoomIn();
  void zoomOut();

 protected:
  void wheelEvent(QWheelEvent* e) override;
  void paintEvent(QPaintEvent* event) override;
};
