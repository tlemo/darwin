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

#pragma once

#include <QFrame>
#include <QPointF>
#include <QRectF>
#include <QTransform>

namespace core_ui {

//! A reusable canvas with support for logical coordinates and auto-scalling the content
//! 
//! Once the logical viewport rectangle is defined, this class maintains the
//! transformations to/from logical viewport coordinates such that the entire viewport
//! is always visible.
//! 
//! Canvas supports both orientations of the y-axis:
//!   - pointing down (default QPainter coordinates)
//!   - pointing up (traditional math/physics Cartesian coordinates)
//! 
//! Using coordinates with y "pointing up" requires extra care, see Canvas::setViewport()
//! 
class Canvas : public QFrame {
 public:
  //! Creates a new Canvas
  explicit Canvas(QWidget* parent);

  //! Border size value (in pixels)
  int borderSize() const { return border_size_; }
  
  //! Sets a new border size (pixels)
  void setBorderSize(int border_size);

  //! Current viewport rectangle (in logical coordinates)
  const QRectF& viewport() const { return viewport_rect_; }
  
  //! Sets the viewport rectangle (logical coordinates)
  //! 
  //! In order to use the traditional Cartesian coordinates, with the y-axis pointing up,
  //! the height of the viewport rectangle should have a negative value. The top_left 
  //! viewport corner should still use the **top** y coordinate. For example, in order
  //! to define a (0,0) -> (10,5) viewport you would call:
  //! 
  //! ```cpp
  //! setViewport(QRectF(0, 5, 10, -5));
  //! ```
  //! 
  //! \warning When using coordinates where y "points up", the text will also be 
  //!   flipped upside down. To render text properly, the viewport transformation must
  //!   be temporary modified to flip the y-coordinate.
  //! 
  void setViewport(const QRectF& viewport_rect);
  
  //! Sets the viewport rectangle
  //! \note Convenience overload, calls setViewport(const QRect&)
  void setViewport(const QPointF& top_left, const QPointF& bottom_right);

  //! Logical viewport coordinates to client window coordinates
  const QTransform& transformFromViewport() const;
  
  //! Client window coordinates to logical viewport coordinates
  const QTransform& transformToViewport() const;

  //! Returns the current scale factor
  double scale() const;

 protected:
  void resizeEvent(QResizeEvent* event) override;

 private:
  void updateTransformations() const;

 private:
  // fixed border around the viewport, in pixels
  int border_size_ = 0;

  // logical viewport coordinates
  QRectF viewport_rect_{ 0, 0, 1, 1 };

  // cached, lazy evaluated transformations
  mutable bool valid_transformations_ = false;
  mutable QTransform transform_from_viewport_;
  mutable QTransform transform_to_viewport_;
  mutable double scale_ = 0;
};

}  // namespace core_ui
