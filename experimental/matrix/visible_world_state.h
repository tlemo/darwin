
#pragma once

#include <third_party/box2d/box2d.h>

#include <vector>

namespace vis {

struct Edge {
  b2Vec2 a;
  b2Vec2 b;
  b2Color color;
};

struct Circle {
  b2Vec2 center;
  float radius;
  b2Color color;
};

struct Polygon {
  std::vector<b2Vec2> points;
  b2Color color;
};

struct Object {
  float radius;
  b2Transform xf;
  b2Vec2 center;
  std::vector<Edge> edges;
  std::vector<Circle> circles;
  std::vector<Polygon> polygons;
  b2Color base_color;

  b2Vec2 worldPoint(const b2Vec2& local_point) const { return b2Mul(xf, local_point); }
};

using World = std::vector<Object>;

}  // namespace vis
