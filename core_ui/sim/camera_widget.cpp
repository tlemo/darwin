// Copyright The Darwin Neuroevolution Framework Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "camera_widget.h"

#include <QImage>
#include <QPainter>
#include <QRgb>

namespace physics_ui {

void CameraWidget::setCamera(const sim::Camera* camera) {
  camera_ = camera;
  update();
}

void CameraWidget::paintEvent(QPaintEvent* event) {
  QWidget::paintEvent(event);
  if (camera_ != nullptr) {
    // create the camera images
    const auto camera_image = camera_->render();
    QImage color_image(int(camera_image.size()), 1, QImage::Format_RGB32);
    QImage depth_image(int(camera_image.size()), 1, QImage::Format_RGB32);
    auto color_pixels = reinterpret_cast<QRgb*>(color_image.scanLine(0));
    auto depth_pixels = reinterpret_cast<QRgb*>(depth_image.scanLine(0));
    for (int i = 0; i < camera_image.size(); ++i) {
      const auto& ci = camera_image[i];
      const float shade = 1.0f - ci.depth;
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
}

}  // namespace physics_ui
