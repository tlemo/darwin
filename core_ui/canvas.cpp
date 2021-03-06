// Copyright 2018 The Darwin Neuroevolution Framework Authors.
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

#include "canvas.h"

#include <core/utils.h>

#include <math.h>

namespace core_ui {

Canvas::Canvas(QWidget* parent) : QFrame(parent) {}

void Canvas::setBorderSize(int border_size) {
  border_size_ = border_size;
  valid_transformations_ = false;
  update();
}

void Canvas::setViewport(const QRectF& viewport_rect, bool auto_update) {
  Q_ASSERT(viewport_rect.width() > 0);
  Q_ASSERT(viewport_rect.height() != 0);
  viewport_rect_ = viewport_rect;
  valid_transformations_ = false;
  if (auto_update) {
    update();
  }
}

void Canvas::setViewport(const QPointF& top_left, const QPointF& bottom_right) {
  setViewport(QRectF(top_left, bottom_right));
}

const QTransform& Canvas::transformFromViewport() const {
  updateTransformations();
  return transform_from_viewport_;
}

const QTransform& Canvas::transformToViewport() const {
  updateTransformations();
  return transform_to_viewport_;
}

double Canvas::scale() const {
  updateTransformations();
  Q_ASSERT(scale_ > 0);
  return scale_;
}

void Canvas::resizeEvent(QResizeEvent* event) {
  QFrame::resizeEvent(event);
  valid_transformations_ = false;
}

void Canvas::updateTransformations() const {
  if (valid_transformations_)
    return;

  // get the client area
  QRectF client_rect = rect();
  CHECK(client_rect.width() > border_size_ * 2);
  CHECK(client_rect.height() > border_size_ * 2);
  client_rect.adjust(border_size_, border_size_, -border_size_, -border_size_);

  // adjust the width or the height to keep the target ratio
  const bool flip_y = viewport_rect_.height() < 0;
  const double target_ratio = fabs(viewport_rect_.width() / viewport_rect_.height());
  if (client_rect.width() / client_rect.height() > target_ratio)
    client_rect.setWidth(client_rect.height() * target_ratio);
  else
    client_rect.setHeight(client_rect.width() / target_ratio);

  client_rect.moveCenter(rect().center());

  scale_ = client_rect.width() / viewport_rect_.width();
  auto viewport_origin = viewport_rect_.topLeft() * scale_;
  if (flip_y) {
    viewport_origin.setY(-viewport_origin.y());
  }
  auto offset = client_rect.topLeft() - viewport_origin;

  // build the transformations
  transform_from_viewport_.reset();
  transform_from_viewport_.translate(offset.x(), offset.y());
  transform_from_viewport_.scale(scale_, (flip_y ? -scale_ : scale_));
  transform_to_viewport_ = transform_from_viewport_.inverted();

  valid_transformations_ = true;
}

}  // namespace core_ui
