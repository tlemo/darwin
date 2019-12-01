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

#include <third_party/tinyspline/tinyspline.h>

#include <tests/testcase_output.h>
#include <third_party/gtest/gtest.h>

#include <vector>
using namespace std;

namespace tinyspline_tests {

// Sample code from the project's page: https://msteinbeck.github.io/tinyspline
TEST(TinySplineTest, Examples) {
  core_test::TestCaseOutput output;

  // Create a cubic spline with 7 control points in 2D using
  // a clamped knot vector. This call is equivalent to:
  // tinyspline::BSpline spline(7, 2, 3, TS_CLAMPED);
  tinyspline::BSpline spline(7);

  // Setup control points.
  std::vector<tinyspline::real> ctrlp = spline.controlPoints();
  ctrlp[0] = -1.75;  // x0
  ctrlp[1] = -1.0;   // y0
  ctrlp[2] = -1.5;   // x1
  ctrlp[3] = -0.5;   // y1
  ctrlp[4] = -1.5;   // x2
  ctrlp[5] = 0.0;    // y2
  ctrlp[6] = -1.25;  // x3
  ctrlp[7] = 0.5;    // y3
  ctrlp[8] = -0.75;  // x4
  ctrlp[9] = 0.75;   // y4
  ctrlp[10] = 0.0;   // x5
  ctrlp[11] = 0.5;   // y5
  ctrlp[12] = 0.5;   // x6
  ctrlp[13] = 0.0;   // y6
  spline.setControlPoints(ctrlp);

  // Evaluate `spline` at u = 0.4 using 'eval'.
  std::vector<tinyspline::real> result = spline.eval(0.4).result();
  fprintf(output, "x=%.2f, y=%.2f\n", result[0], result[1]);

  // Derive `spline` and subdivide it into a sequence of Bezier curves.
  tinyspline::BSpline beziers = spline.derive().toBeziers();

  // Evaluate `beziers` at u = 0.3 using '()' instead of 'eval'.
  result = beziers(0.3).result();
  fprintf(output, "x=%.2f, y=%.2f\n", result[0], result[1]);
}

TEST(TinySplineTest, ClosedCurve) {
  core_test::TestCaseOutput output;

  struct Point {
    double x;
    double y;
  };

  const vector<Point> control_points = {
    { -10, -10 }, { 10, -10 }, { 10, 10 }, { 0, 5 }, { -10, 10 }
  };
  
  const size_t n = control_points.size() + 3;
  tinyspline::BSpline spline(n, 2, 3, TS_OPENED);
  auto cp = spline.controlPoints();
  for (size_t i = 0; i < n; ++i) {
    cp[i * 2 + 0] = control_points[i % control_points.size()].x;
    cp[i * 2 + 1] = control_points[i % control_points.size()].y;
  }
  spline.setControlPoints(cp);
  
  constexpr size_t kSamples = 10;
  const auto samples = spline.sample(kSamples);
  for (size_t i = 0; i < kSamples; ++i) {
    const double x = samples[i * 2 + 0];
    const double y = samples[i * 2 + 1];
    fprintf(output, "%3zu: %.2f, %.2f\n", i, x, y);
  }
}

}  // namespace tinyspline_tests
