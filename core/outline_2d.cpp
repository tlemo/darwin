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

#include "outline_2d.h"

#include <core/utils.h>
#include <third_party/tinyspline/tinyspline.h>

namespace math {

Outline::Outline(const Polygon& control_points, int resolution) {
  // create the spline (as a closed curve)
  const size_t n = control_points.size() + 3;
  tinyspline::BSpline spline(n, 2, 3, TS_OPENED);
  auto cp = spline.controlPoints();
  for (size_t i = 0; i < n; ++i) {
    cp[i * 2 + 0] = control_points[i % control_points.size()].x;
    cp[i * 2 + 1] = control_points[i % control_points.size()].y;
  }
  spline.setControlPoints(cp);

  // get 'resolution' sample points
  // (we defined a closed curve and first/last points overlap, so drop the last one)
  auto samples = spline.sample(resolution + 1);
  samples.pop_back();

  // create the outline polygon
  Polygon polygon(resolution);
  for (size_t i = 0; i < resolution; ++i) {
    polygon[i].x = samples[i * 2 + 0];
    polygon[i].y = samples[i * 2 + 1];
  }

  createNodes(polygon);
}

// detects and eliminates the self-intersections in a polygon
static Polygon resolveSelfIntersections(Polygon polygon) {
  while (polygon.size() > 3) {
    const size_t count = polygon.size();
    const size_t lookahead = count / 4;
    bool self_intersect = false;
    for (size_t i = 0; i < count && !self_intersect; ++i) {
      const auto& a1 = polygon[i];
      const auto& a2 = polygon[(i + 1) % count];
      for (size_t j = 0; j < lookahead; ++j) {
        const auto& b1 = polygon[(i + 2 + j) % count];
        const auto& b2 = polygon[(i + 3 + j) % count];
        const auto r = intersect(a1, a2, b1, b2);
        if (r.a > 0 && r.a < 1 && r.b > 0 && r.b < 1) {
          // set point[i + 1] to the intersection point
          polygon[(i + 1) % count] = a1 + (a2 - a1) * r.a;
          // erase all the points [i + 2 .. i + 2 + j]
          const size_t del_first = (i + 2) % count;
          const size_t del_last = (i + 2 + j) % count;
          const auto it = polygon.begin();
          if (del_first <= del_last) {
            polygon.erase(it + del_first, it + del_last + 1);
          } else {
            polygon.erase(it + del_first, polygon.end());
            polygon.erase(it, it + del_last + 1);
          }
          self_intersect = true;
          break;
        }
      }
    }
    if (!self_intersect) {
      break;
    }
  }
  return polygon;
}

Outline Outline::offset(double offset) const {
  Polygon polygon(nodes_.size());
  for (size_t i = 0; i < nodes_.size(); ++i) {
    polygon[i] = nodes_[i].offset(offset);
  }

  Outline offset_outline;
  offset_outline.createNodes(resolveSelfIntersections(polygon));
  return offset_outline;
}

// space the nodes at a uniform distance from each other
Outline Outline::makeEquidistant() const {
  if (nodes_.size() < 3) {
    return *this;
  }

  const size_t nodes_count = nodes_.size();

  // calculate the outline length
  double total_length = 0;
  for (size_t i = 0; i < nodes_count; ++i) {
    const size_t next_i = (i + 1) % nodes_count;
    total_length += (nodes_[next_i].p - nodes_[i].p).length();
  }

  const double segment_length = total_length / nodes_count;

  Polygon polygon;
  polygon.push_back(nodes_[0].p);
  double reminder = 0;
  for (size_t i = 0; i < nodes_count; ++i) {
    const size_t next_i = (i + 1) % nodes_count;
    Vector2d v = nodes_[next_i].p - nodes_[i].p;
    const double length = v.length();
    v = v / length;
    CHECK(reminder < segment_length);
    double offset = segment_length - reminder;
    reminder += length;
    while (reminder >= segment_length && polygon.size() < nodes_count) {
      polygon.push_back(nodes_[i].p + v * offset);
      offset += segment_length;
      reminder -= segment_length;
    }
  }

  Outline new_outline;
  CHECK(polygon.size() == nodes_count);
  new_outline.createNodes(polygon);
  return new_outline;
}

Polygon Outline::toPolygon() const {
  Polygon polygon(nodes_.size());
  for (size_t i = 0; i < nodes_.size(); ++i) {
    polygon[i] = nodes_[i].p;
  }
  return polygon;
}

void Outline::createNodes(const Polygon& polygon) {
  const size_t nodes_count = polygon.size();

  // calculate the segment normals
  vector<Vector2d> sn(nodes_count);
  for (size_t i = 0; i < nodes_count; ++i) {
    const auto& start = polygon[i];
    const auto& end = polygon[(i + 1) % nodes_count];
    const auto v = end - start;
    sn[i] = Vector2d(v.y, -v.x).normalized();
  }

  // set the outline nodes (position and normal)
  nodes_.resize(nodes_count);
  for (size_t i = 0; i < nodes_count; ++i) {
    const size_t next_i = (i + 1) % nodes_count;

    // calculate the node normal
    //  - size appropriately for an offset = 1.0, not length = 1.0
    //  - the length is also capped (to compensate for sharp corner angles)
    auto n = (sn[i] + sn[next_i]) * 0.5;
    n = n / (n * n);
    const auto normal_length = n.length();
    constexpr double kMaxNormalLength = 1.5;
    if (normal_length > kMaxNormalLength) {
      n = n * (kMaxNormalLength / normal_length);
    }

    nodes_[next_i].p = polygon[next_i];
    nodes_[next_i].n = n;
  }
}

}  // namespace math
