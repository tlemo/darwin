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

#include <core/utils.h>
#include <core/parallel_for_each.h>

#include <third_party/gtest/gtest.h>

#include <atomic>
#include <vector>
using namespace std;

namespace parallel_for_tests {

TEST(ParallelForTest, Basic) {
  vector<int> array(5);

  // parallel-for-loop
  pp::for_each(array, [](int index, int& value) { value += index; });

  // validation
  for (int i = 0; i < array.size(); ++i) {
    EXPECT_EQ(array[i], i);
  }
}

static void parallelForLoop(int array_size) {
  vector<int> array(array_size);

  // initialize with [1..size]
  for (int i = 0; i < array.size(); ++i)
    array[i] = i + 1;

  atomic<int64_t> sum = 0;

  // parallel-for-loop
  pp::for_each(array, [&](int index, int& value) {
    sum += value;
    value = -index;
  });

  // validation
  const int64_t n = array_size;
  EXPECT_EQ(sum, (n * (n + 1)) / 2);
  for (int i = 0; i < array.size(); ++i) {
    EXPECT_EQ(array[i], -i);
  }
}

TEST(ParallelForTest, SmallArrays) {
  for (int array_size = 0; array_size < 100; ++array_size) {
    parallelForLoop(array_size);
  }
}

TEST(ParallelForTest, MediumArrays) {
  constexpr int kBaseSize = 25000;
  for (int array_size = kBaseSize; array_size < kBaseSize + 32; ++array_size) {
    parallelForLoop(array_size);
  }
}

TEST(ParallelForTest, LargeArrays) {
  parallelForLoop(1000000);
}

}  // namespace parallel_for_tests
