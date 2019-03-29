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

#include <populations/cne/cne.h>

#include <third_party/gtest/gtest.h>

namespace cne_crossover_tests {

struct CneCrossoverTest : public testing::TestWithParam<cne::CrossoverOp> {
  CneCrossoverTest() { cne::g_config.crossover_operator = GetParam(); }
};

TEST_P(CneCrossoverTest, SmokeTestSingleElement) {
  constexpr size_t kRows = 1;
  constexpr size_t kCols = 1;
  constexpr float kPreference = 0.1f;

  ann::Matrix child(kRows, kCols);
  ann::Matrix parent1(kRows, kCols);
  ann::Matrix parent2(kRows, kCols);

  cne::crossoverOperator(child, parent1, parent2, kPreference);
}

TEST_P(CneCrossoverTest, SmokeTestColumn) {
  constexpr size_t kRows = 5;
  constexpr size_t kCols = 1;
  constexpr float kPreference = 0.3f;

  ann::Matrix child(kRows, kCols);
  ann::Matrix parent1(kRows, kCols);
  ann::Matrix parent2(kRows, kCols);

  cne::crossoverOperator(child, parent1, parent2, kPreference);
}

TEST_P(CneCrossoverTest, SmokeTestRow) {
  constexpr size_t kRows = 1;
  constexpr size_t kCols = 10;
  constexpr float kPreference = 0.9f;

  ann::Matrix child(kRows, kCols);
  ann::Matrix parent1(kRows, kCols);
  ann::Matrix parent2(kRows, kCols);

  cne::crossoverOperator(child, parent1, parent2, kPreference);
}

TEST_P(CneCrossoverTest, SmokeTestGeneric) {
  constexpr size_t kRows = 7;
  constexpr size_t kCols = 16;
  constexpr float kPreference = 0.5f;

  ann::Matrix child(kRows, kCols);
  ann::Matrix parent1(kRows, kCols);
  ann::Matrix parent2(kRows, kCols);

  cne::crossoverOperator(child, parent1, parent2, kPreference);
}

INSTANTIATE_TEST_CASE_P(All,
                        CneCrossoverTest,
                        testing::ValuesIn({
                            cne::CrossoverOp::Mix,
                            cne::CrossoverOp::Split,
                            cne::CrossoverOp::RowSplit,
                            cne::CrossoverOp::ColSplit,
                            cne::CrossoverOp::RowOrColSplit,
                            cne::CrossoverOp::PrefRowSplit,
                            cne::CrossoverOp::PrefAverage,
                            cne::CrossoverOp::RowMix,
                            cne::CrossoverOp::ColMix,
                            cne::CrossoverOp::RowOrColMix,
                            cne::CrossoverOp::Quadrants,
                            cne::CrossoverOp::BestParent,
                            cne::CrossoverOp::Randomize,
                        }));

}  // namespace cne_crossover_tests
