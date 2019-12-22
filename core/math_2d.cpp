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

#include "math_2d.h"

namespace math {

// The intersection is the solution to:
//   x = x' => x0 + dx * a = x0' + dx' * a'
//   y = y' => y0 + dy * a = y0' + dy' * a'
//
Intersection2d intersect(const Vector2d& a1,
                         const Vector2d& a2,
                         const Vector2d& b1,
                         const Vector2d& b2) {
  const Scalar x = a1.x;
  const Scalar y = a1.y;
  const Scalar dx = a2.x - x;
  const Scalar dy = a2.y - y;

  const Scalar x2 = b1.x;
  const Scalar y2 = b1.y;
  const Scalar dx2 = b2.x - x2;
  const Scalar dy2 = b2.y - y2;

  const Scalar w = dy * dx2 - dx * dy2;

  if (w == 0) {
    return { kInfinity, kInfinity };
  } else {
    const Scalar a = (dx2 * (y2 - y) - dy2 * (x2 - x)) / w;
    const Scalar b = (dx2 != 0) ? ((x + a * dx) - x2) / dx2 : ((y + a * dy) - y2) / dy2;
    return { a, b };
  }
}

void HMatrix2d::setIdentity() {
  setZero();
  m[0][0] = 1;
  m[1][1] = 1;
  m[2][2] = 1;
}

void HMatrix2d::setTranslate(Scalar tx, Scalar ty) {
  setIdentity();
  m[0][2] = tx;
  m[1][2] = ty;
}

void HMatrix2d::setScale(Scalar sx, Scalar sy) {
  setZero();
  m[0][0] = sx;
  m[1][1] = sy;
  m[2][2] = 1;
}

void HMatrix2d::setRotation(Scalar angle) {
  const Scalar c = cos(angle);
  const Scalar s = sin(angle);
  setIdentity();
  m[0][0] = c;
  m[0][1] = -s;
  m[1][0] = s;
  m[1][1] = c;
}

HMatrix2d HMatrix2d::operator*(const HMatrix2d& other) {
  HMatrix2d result;
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      for (int k = 0; k < 3; ++k)
        result.m[i][j] += m[i][k] * other.m[k][j];
  return result;
}

Vector2d HMatrix2d::operator*(const Vector2d& v) {
  const Scalar w = m[2][0] * v.x + m[2][1] * v.y + m[2][2];
  return Vector2d((m[0][0] * v.x + m[0][1] * v.y + m[0][2]) / w,
                  (m[1][0] * v.x + m[1][1] * v.y + m[1][2]) / w);
}

}  // namespace math
