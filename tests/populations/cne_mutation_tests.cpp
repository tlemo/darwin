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

namespace cne_mutation_tests {

struct CneMutationTest : public testing::TestWithParam<cne::MutationOp> {
  CneMutationTest() {
    cne::g_config.mutation_operator = GetParam();
    cne::g_config.mutation_chance = 0.8f;
  }
};

TEST_P(CneMutationTest, SmokeTestSingleElement) {
  constexpr size_t kRows = 1;
  constexpr size_t kCols = 1;
  constexpr float kStdDev = 1.0f;
  ann::g_config.mutation_normal_distribution = false;
  ann::Matrix child(kRows, kCols);
  cne::mutationOperator(child, kStdDev);
}

TEST_P(CneMutationTest, SmokeTestColumn) {
  constexpr size_t kRows = 9;
  constexpr size_t kCols = 1;
  constexpr float kStdDev = 2.0f;
  ann::g_config.mutation_normal_distribution = true;
  ann::Matrix child(kRows, kCols);
  cne::mutationOperator(child, kStdDev);
}

TEST_P(CneMutationTest, SmokeTestRow) {
  constexpr size_t kRows = 1;
  constexpr size_t kCols = 2;
  constexpr float kStdDev = 1.0f;
  ann::g_config.mutation_normal_distribution = false;
  ann::Matrix child(kRows, kCols);
  cne::mutationOperator(child, kStdDev);
}

TEST_P(CneMutationTest, SmokeTestGeneric) {
  constexpr size_t kRows = 2;
  constexpr size_t kCols = 17;
  constexpr float kStdDev = 4.0f;
  ann::g_config.mutation_normal_distribution = true;
  ann::Matrix child(kRows, kCols);
  cne::mutationOperator(child, kStdDev);
}

INSTANTIATE_TEST_CASE_P(All,
                        CneMutationTest,
                        testing::ValuesIn({
                            cne::MutationOp::IndividualCells,
                            cne::MutationOp::AllCells,
                            cne::MutationOp::RowOrCol,
                            cne::MutationOp::Row,
                            cne::MutationOp::Col,
                            cne::MutationOp::RowAndCol,
                            cne::MutationOp::SubRect,
                        }));

}  // namespace cne_mutation_tests
