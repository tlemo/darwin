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

#include <core/math_2d.h>

#include <vector>
using namespace std;

namespace math {

//! A list of 2d points
using Polygon = vector<Vector2d>;

//! A spline aproximation
class Outline {
 public:
  struct Node {
    Vector2d p;  //!< position
    Vector2d n;  //!< normal

    //! Returns a point at the specified offset (along the normal)
    Vector2d offset(double offset) const { return p + n * offset; }
  };

 public:
  //! Creates an empty outline
  Outline() = default;

  //! Create a spline outline
  explicit Outline(const Polygon& control_points, int resolution);

  //! Returns the outline nodes
  const vector<Node>& nodes() const { return nodes_; }

  //! Returns a new outline at the specified offset
  Outline offset(double offset) const;

  //! Returns a new outline with the nodes spaced at equal distance from each other
  Outline makeEquidistant() const;

  //! Returns the polygon equivalent
  Polygon toPolygon() const;

  //! Clears the outline
  void clear() { nodes_.clear(); }

  //! Returns true if the outline is empty
  bool empty() const { return nodes_.empty(); }

 private:
  void createNodes(const Polygon& polygon);

 private:
  vector<Node> nodes_;
};

}  // namespace math
