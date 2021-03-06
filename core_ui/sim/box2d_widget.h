// Copyright 2019 The Darwin Neuroevolution Framework Authors.
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

#include <core_ui/canvas.h>
#include <third_party/box2d/box2d.h>

#include <QColor>
#include <QRectF>

namespace physics_ui {

class Box2dSandboxWindow;

//! Custom box2d scene rendering & input processing
class Box2dSceneUi : public QObject {
  Q_OBJECT

 public:
  // rendering
  virtual void render(QPainter& /*painter*/, const QRectF& /*viewport*/) {}

  // mouse
  virtual void mousePressEvent(const QPointF& /*pos*/, QMouseEvent* /*event*/) {}
  virtual void mouseReleaseEvent(const QPointF& /*pos*/, QMouseEvent* /*event*/) {}
  virtual void mouseMoveEvent(const QPointF& /*pos*/, QMouseEvent* /*event*/) {}

  // keyboard
  virtual void keyPressEvent(QKeyEvent* /*event*/) {}
  virtual void keyReleaseEvent(QKeyEvent* /*event*/) {}

  // focus
  virtual void focusInEvent() {}
  virtual void focusOutEvent() {}

  // simulation step
  // TODO: revisit this
  virtual void step() {}

  // help text
  virtual QString help() const { return ""; }

  // optional viewport adjustment
  virtual QRectF adjustViewport(const QRectF& viewport) { return viewport; }

 signals:
  void sigPlayPause();
};

//! Visualization for a b2World, with an optional Box2dSceneUi plugin
//!
//! \warning Box2dWidget does't own either b2World or Box2dSceneUi instances
//!   (these instances must outlive their use in Box2dWidget)
//!
class Box2dWidget : public core_ui::Canvas {
  Q_OBJECT

  const QColor kDefaultBackgroundColor{ 255, 255, 255 };
  const QColor kDefaultViewportColor{ 240, 240, 255 };

 public:
  //! The viewpoint extents rules
  enum class ViewportPolicy {
    UserDefined,    //!< Use the setViewport() to set the extents (default)
    AutoExpanding,  //!< Automatically expand to fit all objects
    AutoFit         //!< Expand or shrink as needed to fit current objects
  };

 public:
  explicit Box2dWidget(QWidget* parent);

  void setWorld(b2World* world);
  void setSceneUi(Box2dSceneUi* scene_ui);

  ViewportPolicy viewportPolicy() const { return viewport_policy_; }
  void setViewportPolicy(ViewportPolicy policy);

  bool debugRender() const { return enable_debug_render_; }
  void setDebugRender(bool enable);

  bool renderLights() const { return render_lights_; }
  void setRenderLights(bool enable);

  void setBackgroundColor(const QColor& color);
  void setViewportColor(const QColor& color);

 signals:
  void sigPlayPause();

 protected:
  void paintEvent(QPaintEvent* event) override;

  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;

  void keyPressEvent(QKeyEvent* event) override;
  void keyReleaseEvent(QKeyEvent* event) override;

  void focusInEvent(QFocusEvent*) override;
  void focusOutEvent(QFocusEvent*) override;

 private:
  void renderDebugLayer(QPainter& painter) const;
  void renderGeneric(QPainter& painter) const;

  void applyViewportPolicy();

 private:
  b2World* world_ = nullptr;
  Box2dSceneUi* scene_ui_ = nullptr;
  ViewportPolicy viewport_policy_ = ViewportPolicy::UserDefined;
  QRectF viewport_reference_;
  bool enable_debug_render_ = true;
  bool render_lights_ = false;
  QColor viewport_color_ = kDefaultViewportColor;
};

}  // namespace physics_ui
