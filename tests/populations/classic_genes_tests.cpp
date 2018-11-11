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
#include <populations/classic/classic.h>
#include <populations/classic/feedforward.h>
#include <populations/classic/full_rnn.h>
#include <populations/classic/lstm.h>
#include <populations/classic/lstm_lite.h>
#include <populations/classic/rnn.h>

#include <third_party/json/json.h>
using nlohmann::json;

#include <tests/testcase_output.h>
#include <third_party/gtest/gtest.h>

namespace classic_genes_tests {

void initializeMatrix(ann::Matrix& m, float start, float step) {
  float value = start;
  for (float& cell : m.values) {
    cell = value;
    value += step;
  }
}

void checkMatrix(const ann::Matrix& m, float start, float step) {
  float value = start;
  for (float cell : m.values) {
    ASSERT_EQ(cell, value);
    value += step;
  }
}

TEST(ClassicGenesTest, FF_Gene_LoadSave) {
  core_test::TestCaseOutput output;

  constexpr size_t kInputs = 3;
  constexpr size_t kOutputs = 2;
  classic::feedforward::Gene gene(kInputs, kOutputs);
  initializeMatrix(gene.w, 0, 1);
  json json_obj = gene;
  fprintf(output, "%s", json_obj.dump(2).c_str());
  classic::feedforward::Gene gene_clone = json_obj;
  checkMatrix(gene_clone.w, 0, 1);
}

TEST(ClassicGenesTest, Full_RNN_Gene_LoadSave) {
  core_test::TestCaseOutput output;

  constexpr size_t kInputs = 3;
  constexpr size_t kOutputs = 2;
  classic::full_rnn::Gene gene(kInputs, kOutputs);
  initializeMatrix(gene.w, 0, 1);
  initializeMatrix(gene.rw, 100, 1);
  json json_obj = gene;
  fprintf(output, "%s", json_obj.dump(2).c_str());
  classic::full_rnn::Gene gene_clone = json_obj;
  checkMatrix(gene_clone.w, 0, 1);
  checkMatrix(gene_clone.rw, 100, 1);
}

TEST(ClassicGenesTest, LSTM_Gene_LoadSave) {
  core_test::TestCaseOutput output;

  constexpr size_t kInputs = 3;
  constexpr size_t kOutputs = 2;
  classic::lstm::Gene gene(kInputs, kOutputs);
  initializeMatrix(gene.w, 0, 1);
  initializeMatrix(gene.lw, 100, 1);
  json json_obj = gene;
  fprintf(output, "%s", json_obj.dump(2).c_str());
  classic::lstm::Gene gene_clone = json_obj;
  checkMatrix(gene_clone.w, 0, 1);
  checkMatrix(gene_clone.lw, 100, 1);
}

TEST(ClassicGenesTest, LSTM_Lite_Gene_LoadSave) {
  core_test::TestCaseOutput output;

  constexpr size_t kInputs = 3;
  constexpr size_t kOutputs = 2;
  classic::lstm_lite::Gene gene(kInputs, kOutputs);
  initializeMatrix(gene.w, 0, 1);
  initializeMatrix(gene.lw, 100, 1);
  json json_obj = gene;
  fprintf(output, "%s", json_obj.dump(2).c_str());
  classic::lstm_lite::Gene gene_clone = json_obj;
  checkMatrix(gene_clone.w, 0, 1);
  checkMatrix(gene_clone.lw, 100, 1);
}

TEST(ClassicGenesTest, RNN_Gene_LoadSave) {
  core_test::TestCaseOutput output;

  constexpr size_t kInputs = 3;
  constexpr size_t kOutputs = 2;
  classic::rnn::Gene gene(kInputs, kOutputs);
  initializeMatrix(gene.w, 0, 1);
  initializeMatrix(gene.rw, 100, 1);
  json json_obj = gene;
  fprintf(output, "%s", json_obj.dump(2).c_str());
  classic::rnn::Gene gene_clone = json_obj;
  checkMatrix(gene_clone.w, 0, 1);
  checkMatrix(gene_clone.rw, 100, 1);
}

}  // namespace classic_genes_tests
