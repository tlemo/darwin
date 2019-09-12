
#include "camera_widget.h"
#include "camera.h"

#include <QImage>
#include <QPainter>
#include <QRgb>

CameraWidget::CameraWidget(QWidget* parent, const phys::Camera* camera)
    : QWidget(parent), camera_(camera) {
  setMinimumSize(64, 64);
  setMaximumSize(4096, 64);
}

void CameraWidget::paintEvent(QPaintEvent*) {
  // create the camera images
  const auto camera_image = camera_->render();
  QImage color_image(int(camera_image.size()), 1, QImage::Format_RGB32);
  QImage depth_image(int(camera_image.size()), 1, QImage::Format_RGB32);
  auto color_pixels = reinterpret_cast<QRgb*>(color_image.scanLine(0));
  auto depth_pixels = reinterpret_cast<QRgb*>(depth_image.scanLine(0));
  for (int i = 0; i < camera_image.size(); ++i) {
    const auto& ci = camera_image[i];
    const float shade = 1.0f - ci.distance;
    color_pixels[i] = QColor::fromRgbF(ci.r, ci.g, ci.b).rgb();
    depth_pixels[i] = QColor::fromRgbF(shade, shade, shade).rgb();
  }

  // calculate the layout of the top/bottom halfs
  const auto half_height = rect().height() / 2;
  auto top_half = rect();
  top_half.setHeight(half_height);
  auto bottom_half = rect();
  bottom_half.moveTop(half_height);

  // render the camera image
  QPainter painter(this);
  painter.drawImage(top_half, color_image);
  painter.drawImage(top_half, depth_image);
}
