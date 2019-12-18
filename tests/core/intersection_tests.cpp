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

#include <core/utils.h>
#include <core/math_2d.h>

#include <third_party/gtest/gtest.h>

namespace intersection_tests {

TEST(IntersectionTest, Degenerate) {
  auto r1 = math::intersect({0, 0}, {0, 0}, {0, 0}, {0, 0});
  EXPECT_EQ(r1.a, math::kInfinity);
  EXPECT_EQ(r1.b, math::kInfinity);
  
  auto r2 = math::intersect({1, 1}, {1, 1}, {-1, -1}, {-1, -1});
  EXPECT_EQ(r2.a, math::kInfinity);
  EXPECT_EQ(r2.b, math::kInfinity);

  auto r3 = math::intersect({+0.0, +0.0}, {-0.0, -0.0}, {0, 0}, {-1, -1});
  EXPECT_EQ(r3.a, math::kInfinity);
  EXPECT_EQ(r3.b, math::kInfinity);
}

TEST(IntersectionTest, Simple) {
  auto r1 = math::intersect({-1, -1}, {1, 1}, {-1, 1}, {1, -1});
  EXPECT_TRUE(r1.a > 0 && r1.a < 1);
  EXPECT_TRUE(r1.b > 0 && r1.b < 1);
  
  auto r2 = math::intersect({0, -1}, {0, 1}, {-1, 0}, {1, 0});
  EXPECT_TRUE(r2.a > 0 && r2.a < 1);
  EXPECT_TRUE(r2.b > 0 && r2.b < 1);

  auto r3 = math::intersect({0, -1}, {0, 1}, {1, 1}, {-1, -1});
  EXPECT_TRUE(r3.a > 0 && r3.a < 1);
  EXPECT_TRUE(r3.b > 0 && r3.b < 1);
}

TEST(IntersectionTest, SmallSlopes) {
  auto r1 = math::intersect({-1, 0}, {1, 0.001}, {1, 0}, {-1, 0.002});
  EXPECT_TRUE(r1.a > 0 && r1.a < 1);
  EXPECT_TRUE(r1.b > 0 && r1.b < 1);
  
  auto r2 = math::intersect({0, 0}, {0.0001, 1}, {0, 1}, {0.01, 0});
  EXPECT_TRUE(r2.a > 0 && r2.a < 1);
  EXPECT_TRUE(r2.b > 0 && r2.b < 1);
}

TEST(IntersectionTest, Partial) {
  auto r1 = math::intersect({1, 1}, {-1, -1}, {-1, 1}, {-0.5, 0.5});
  EXPECT_TRUE(r1.a > 0 && r1.a < 1);
  EXPECT_GT(r1.b, 1);
  
  auto r2 = math::intersect({1, -1}, {0.5, -0.5}, {1, 1}, {-1, -1});
  EXPECT_GT(r2.a, 1);
  EXPECT_TRUE(r2.b > 0 && r2.b < 1);
}

TEST(IntersectionTest, Parallel) {
  // horizontal
  auto r1 = math::intersect({0, 0}, {1, 0}, {0, 1}, {1, 1});
  EXPECT_EQ(r1.a, math::kInfinity);
  EXPECT_EQ(r1.b, math::kInfinity);
  
  // vertical
  auto r2 = math::intersect({-1, -1}, {-1, 1}, {1, 1}, {1, -1});
  EXPECT_EQ(r2.a, math::kInfinity);
  EXPECT_EQ(r2.b, math::kInfinity);

  // arbritrary
  auto r3 = math::intersect({0, 0}, {1, 0.1}, {-1, 0}, {0, 0.1});
  EXPECT_EQ(r3.a, math::kInfinity);
  EXPECT_EQ(r3.b, math::kInfinity);
}

TEST(IntersectionTest, Collinear) {
  auto r1 = math::intersect({0, 0}, {1, 0.5}, {0, 0}, {1, 0.5});
  EXPECT_EQ(r1.a, math::kInfinity);
  EXPECT_EQ(r1.b, math::kInfinity);
  
  auto r2 = math::intersect({0, 0}, {-1, 0}, {0, 0}, {1, 0});
  EXPECT_EQ(r2.a, math::kInfinity);
  EXPECT_EQ(r2.b, math::kInfinity);

  auto r3 = math::intersect({0, 0}, {0, 1}, {0, 2}, {0, 3});
  EXPECT_EQ(r3.a, math::kInfinity);
  EXPECT_EQ(r3.b, math::kInfinity);
}

}  // namespace intersection_tests
