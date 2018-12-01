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

#include <populations/classic/classic.h>

#include <third_party/gtest/gtest.h>

namespace classic_crossover_tests {

struct ClassicCrossoverTest : public testing::TestWithParam<classic::CrossoverOp> {
  ClassicCrossoverTest() {
    classic::g_config.crossover_operator = GetParam();
  }
};

TEST_P(ClassicCrossoverTest, SmokeTestSingleElement) {
  constexpr size_t kRows = 1;
  constexpr size_t kCols = 1;
  constexpr float kPreference = 0.1f;
  
  ann::Matrix child(kRows, kCols);
  ann::Matrix parent1(kRows, kCols);
  ann::Matrix parent2(kRows, kCols);
  
  classic::crossoverOperator(child, parent1, parent2, kPreference);
}

TEST_P(ClassicCrossoverTest, SmokeTestColumn) {
  constexpr size_t kRows = 5;
  constexpr size_t kCols = 1;
  constexpr float kPreference = 0.3f;
  
  ann::Matrix child(kRows, kCols);
  ann::Matrix parent1(kRows, kCols);
  ann::Matrix parent2(kRows, kCols);
  
  classic::crossoverOperator(child, parent1, parent2, kPreference);
}

TEST_P(ClassicCrossoverTest, SmokeTestRow) {
  constexpr size_t kRows = 1;
  constexpr size_t kCols = 10;
  constexpr float kPreference = 0.9f;
  
  ann::Matrix child(kRows, kCols);
  ann::Matrix parent1(kRows, kCols);
  ann::Matrix parent2(kRows, kCols);
  
  classic::crossoverOperator(child, parent1, parent2, kPreference);
}

TEST_P(ClassicCrossoverTest, SmokeTestGeneric) {
  constexpr size_t kRows = 7;
  constexpr size_t kCols = 16;
  constexpr float kPreference = 0.5f;
  
  ann::Matrix child(kRows, kCols);
  ann::Matrix parent1(kRows, kCols);
  ann::Matrix parent2(kRows, kCols);
  
  classic::crossoverOperator(child, parent1, parent2, kPreference);
}

INSTANTIATE_TEST_CASE_P(All,
                        ClassicCrossoverTest,
                        testing::ValuesIn({
                            classic::CrossoverOp::Mix,
                            classic::CrossoverOp::Split,
                            classic::CrossoverOp::RowSplit,
                            classic::CrossoverOp::ColSplit,
                            classic::CrossoverOp::RowOrColSplit,
                            classic::CrossoverOp::PrefRowSplit,
                            classic::CrossoverOp::PrefAverage,
                            classic::CrossoverOp::RowMix,
                            classic::CrossoverOp::ColMix,
                            classic::CrossoverOp::RowOrColMix,
                            classic::CrossoverOp::Quadrants,
                            classic::CrossoverOp::BestParent,
                            classic::CrossoverOp::Randomize,
                        }));

}  // namespace classic_crossover_tests
