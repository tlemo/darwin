
#pragma once

#include "camera.h"

#include <QWidget>

class CameraWidget : public QWidget {
 public:
  explicit CameraWidget(QWidget* parent, const phys::Camera* camera);

 protected:
  void paintEvent(QPaintEvent* event) override;

 private:
  const phys::Camera* camera_ = nullptr;
};
