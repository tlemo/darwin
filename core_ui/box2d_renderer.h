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

#include <third_party/box2d/box2d.h>

#include <QPainter>

namespace core_ui {

//! A QPainter-based implementation of Box2d debug draw interface (b2Draw)
class Box2dRenderer : public b2Draw {
 public:
  explicit Box2dRenderer(QPainter* painter) : painter_(painter) {}

  // draw a closed polygon provided in CCW order
  void DrawPolygon(const b2Vec2* vertices,
                   int32 vertexCount,
                   const b2Color& color) override;

  // draw a solid closed polygon provided in CCW order
  void DrawSolidPolygon(const b2Vec2* vertices,
                        int32 vertexCount,
                        const b2Color& color) override;

  void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color) override;

  void DrawSolidCircle(const b2Vec2& center,
                       float32 radius,
                       const b2Vec2& axis,
                       const b2Color& color) override;

  void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override;

  void DrawTransform(const b2Transform& xf) override;

  void DrawPoint(const b2Vec2& p, float32 size, const b2Color& color) override;

 private:
  QPainter* painter_ = nullptr;
};

}  // namespace core_ui
