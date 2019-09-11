
#pragma once

#include <QWidget>

class CameraWidget : public QWidget {
 public:
  explicit CameraWidget(QWidget* parent);

 protected:
  void paintEvent(QPaintEvent* event) override;
};
