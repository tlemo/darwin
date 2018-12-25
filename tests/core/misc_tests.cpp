// Copyright 2018 The Darwin Neuroevolution Framework Authors.
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

#include <third_party/gtest/gtest.h>

#include <random>

namespace misc_tests {

// a quick sanity check for std::random_device
//
// 1. https://en.cppreference.com/w/cpp/numeric/random/random_device
// 2. http://www.pcg-random.org/posts/cpps-random_device.html
//
// current versions of MinGW ship with a broken implementation which returns
// a deterministic sequence of values, which defeats the random_device's purpose
// (https://sourceforge.net/p/mingw-w64/bugs/338)
//
// NOTE: yes, technically std::random_device is permitted to be pseudo-random,
//  although there's no excuse to degrade to that on a general purpose platform
//
TEST(StandardLibraryTest, RandomDevice) {
  std::random_device rd_1;
  std::random_device rd_2;
  
  // with a decent random_device implementation, the chance of a collision
  // should be 1 / 2^32 (assuming 32bit integers)
  EXPECT_NE(rd_1(), rd_2()) << "Sorry, you have a useless std::random_device";
}

}  // namespace misc_tests

