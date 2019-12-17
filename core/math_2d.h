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

#include <cmath>
#include <cstring>
#include <limits>
using namespace std;

namespace math {

//! The scalar type used with general-purpose math utilities
using Scalar = double;

//! [Infinity](https://en.cppreference.com/w/cpp/types/numeric_limits/infinity)
constexpr Scalar kInfinity = numeric_limits<Scalar>::infinity();

//! [Epsilon](https://en.cppreference.com/w/cpp/types/numeric_limits/epsilon)
constexpr Scalar kEpsilon = numeric_limits<Scalar>::epsilon();

//! [Pi](https://en.wikipedia.org/wiki/Pi)
constexpr Scalar kPi = 3.141592653589;

//! [Radian](https://en.wikipedia.org/wiki/Radian) to
//! [Degree](https://en.wikipedia.org/wiki/Degree_(angle)) conversion
constexpr Scalar radiansToDegrees(Scalar radians) {
  return radians / kPi * 180;
}

//! [Degree](https://en.wikipedia.org/wiki/Degree_(angle)) to
//! [Radian](https://en.wikipedia.org/wiki/Radian) conversion
constexpr Scalar degreesToRadians(Scalar degrees) {
  return degrees / 180 * kPi;
}

//! A basic 2D vector
struct Vector2d {
  Scalar x;  //!< x coordinate
  Scalar y;  //!< y coordinate

  //! Constructs a zero-vector
  Vector2d() : x(0), y(0) {}

  //! Constructs a vector with the specified components
  Vector2d(Scalar x, Scalar y) : x(x), y(y) {}

  //! The magnitude (length) of the vector
  Scalar length() const { return sqrt(x * x + y * y); }

  //! Returns a normalized (length = 1) version of this vector
  Vector2d normalized() const { return *this / length(); }

  //! Vector addition
  Vector2d operator+(const Vector2d& v) const { return Vector2d(x + v.x, y + v.y); }

  //! Vector subtraction
  Vector2d operator-(const Vector2d& v) const { return Vector2d(x - v.x, y - v.y); }

  //! Scalar multiplication
  Vector2d operator*(Scalar s) const { return Vector2d(x * s, y * s); }

  //! Scalar division
  Vector2d operator/(Scalar s) const { return Vector2d(x / s, y / s); }

  //! [Dot product](https://en.wikipedia.org/wiki/Dot_product)
  Scalar operator*(const Vector2d& v) const { return x * v.x + y * v.y; }
};

//! The intersection of two 2d segments
//!
//! The intersection values are offsets relative to the segment:
//! 0 is the segment start, 1 to the segment end (values less than 0
//! represent points before the segment start, greater than 1 after the segment end)
//!
struct Intersection2d {
  Scalar a = 0;  //!< The offset relative to the first segment
  Scalar b = 0;  //!< The offset relative to the second segment
};

//! Calculate the intersection between 2 segments
//!
//! The first segment is A:[a1, a2]
//! The second segment is B:[b1, b2]
//!
//! If the segments are parallel, kInfinity is returned
//!
Intersection2d intersect(const Vector2d& a1,
                         const Vector2d& a2,
                         const Vector2d& b1,
                         const Vector2d& b2);

//! A 3x3 [homogeneous](https://en.wikipedia.org/wiki/Homogeneous_coordinates)
//! transformation matrix
struct HMatrix2d {
  //! Matrix elements
  Scalar m[3][3];

  //! Constructs a zero-matrix
  HMatrix2d() { setZero(); }

  //! Resets all the elements to zero
  void setZero() { ::memset(m, 0, sizeof(m)); }

  //! Resets the matrix to [identity](https://en.wikipedia.org/wiki/Identity_matrix)
  void setIdentity();

  //! Resets the matrix to the specified translation transformation
  void setTranslate(Scalar tx, Scalar ty);

  //! Resets the matrix to the specified scale transformation
  void setScale(Scalar sx, Scalar sy);

  //! Resets the matrix to the specified rotation transformation
  void setRotation(Scalar angle);

  //! Matrix multiplication
  HMatrix2d operator*(const HMatrix2d& other);

  //! Matrix x Vector multiplication
  Vector2d operator*(const Vector2d& v);
};

}  // namespace math
