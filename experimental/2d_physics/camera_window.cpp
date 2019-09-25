
#include "camera_window.h"
#include "sandbox_window.h"

#include <QImage>
#include <QPainter>
#include <QRgb>

CameraWindow::CameraWindow(QWidget* parent, const phys::Camera* camera)
    : ToolWindow(parent), camera_(camera) {
  setWindowTitle("Camera");
  setMinimumSize(64, 64);
  setMaximumSize(4096, 64);
  setContentsMargins(5, 5, 5, 5);
}

void CameraWindow::paintEvent(QPaintEvent*) {
  if (camera_ == nullptr) {
    return;
  }

  // create the camera images
  const auto camera_image = camera_->render();
  QImage color_image(int(camera_image.size()), 1, QImage::Format_RGB32);
  QImage depth_image(int(camera_image.size()), 1, QImage::Format_RGB32);
  auto color_pixels = reinterpret_cast<QRgb*>(color_image.scanLine(0));
  auto depth_pixels = reinterpret_cast<QRgb*>(depth_image.scanLine(0));
  for (int i = 0; i < camera_image.size(); ++i) {
    const auto& ci = camera_image[i];
    const float shade = 1.0f - ci.distance;
    color_pixels[i] = QColor::fromRgbF(ci.color.r, ci.color.g, ci.color.b).rgb();
    depth_pixels[i] = QColor::fromRgbF(shade, shade, shade).rgb();
  }

  // calculate the layout of the top/bottom halfs
  auto top_half = contentsRect();
  const auto half_height = top_half.height() / 2;
  top_half.setHeight(half_height);
  auto bottom_half = top_half;
  bottom_half.translate(0, half_height);

  // render the camera image
  QPainter painter(this);
  painter.drawImage(top_half, color_image);
  painter.drawImage(bottom_half, depth_image);
}

void CameraWindow::onSandboxChange(SandboxWindow* sandbox_window) {
  if (sandbox_window != nullptr) {
    camera_ = sandbox_window->scenePackage().scene->camera();
  } else {
    camera_ = nullptr;
  }
  update();
}
