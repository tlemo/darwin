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

namespace core_ui {

class Box2dSandboxWindow;

//! Custom box2d scene rendering & input processing
class Box2dSceneUi : public QObject {
  Q_OBJECT

 public:
  // initial setup
  virtual void init(Box2dSandboxWindow* /*sandbox_window*/) {}

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
  virtual void step() {}

 signals:
  void sigPlayPause();
};

class Box2dWidget : public core_ui::Canvas {
  Q_OBJECT

  const QColor kBackgroundColor{ 255, 255, 255 };
  const QColor kViewportColor{ 240, 240, 255 };

 public:
  explicit Box2dWidget(QWidget* parent);

  void setWorld(b2World* world, const QRectF& viewport);
  void setSceneUi(Box2dSceneUi* scene_ui);

  bool debugRender() const { return enable_debug_render_; }

  void setDebugRender(bool enable);

 signals:
  void sigPlayPause();

 private:
  void paintEvent(QPaintEvent* event) override;

  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  
  void keyPressEvent(QKeyEvent* event) override;
  void keyReleaseEvent(QKeyEvent* event) override;
  
  void focusInEvent(QFocusEvent*) override;
  void focusOutEvent(QFocusEvent*) override;
  
  void renderDebugLayer(QPainter& painter) const;

 private:
  b2World* world_ = nullptr;
  Box2dSceneUi* scene_ui_ = nullptr;
  bool enable_debug_render_ = true;
};

}  // namespace core_ui
