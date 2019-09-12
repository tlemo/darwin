
#include "camera_widget.h"
#include "camera.h"

#include <QPainter>
#include <QImage>
#include <QRgb>

CameraWidget::CameraWidget(QWidget* parent, const phys::Camera* camera)
    : QWidget(parent), camera_(camera) {
  setMinimumSize(64, 64);
  setMaximumSize(4096, 64);
}

void CameraWidget::paintEvent(QPaintEvent*) {
  // create the camera image
  const auto camera_image = camera_->render();
  QImage image(camera_image.size(), 1, QImage::Format_RGB32);
  auto pixels = reinterpret_cast<QRgb*>(image.scanLine(0));
  for(int i = 0; i < camera_image.size(); ++i) {
    const auto& ci = camera_image[i];
    pixels[i] = QColor::fromRgbF(ci.r, ci.g, ci.b).rgb();
  }

  // render the camera image
  QPainter painter(this);
  painter.drawImage(rect(), image);
}
