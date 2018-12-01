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

#include "classic.h"

#include <core/ann_dynamic.h>

namespace classic {

void crossoverOperator(ann::Matrix& child,
                       const ann::Matrix& parent1,
                       const ann::Matrix& parent2,
                       float preference) {
  const size_t rows = child.rows;
  const size_t cols = child.cols;

  CHECK(cols > 0 && rows > 0);
  CHECK(cols == parent1.cols && cols == parent2.cols);
  CHECK(rows == parent1.rows && rows == parent2.rows);

  std::random_device rd;
  std::default_random_engine rnd(rd());
  std::bernoulli_distribution dist_parent(preference);
  std::bernoulli_distribution dist_coin;

  switch (g_config.crossover_operator) {
    case CrossoverOp::Mix: {
      for (size_t i = 0; i < rows; ++i)
        for (size_t j = 0; j < cols; ++j)
          child[i][j] = dist_parent(rnd) ? parent1[i][j] : parent2[i][j];
    } break;

    case CrossoverOp::Split: {
      std::uniform_int_distribution<size_t> dist_row(0, rows - 1);
      std::uniform_int_distribution<size_t> dist_col(0, cols - 1);

      size_t row_split = dist_row(rnd);
      size_t col_split = dist_col(rnd);

      for (size_t i = 0; i < rows; ++i)
        for (size_t j = 0; j < cols; ++j) {
          child[i][j] = (i < row_split && j < col_split) ? parent1[i][j] : parent2[i][j];
        }
    } break;

    case CrossoverOp::RowSplit: {
      std::uniform_int_distribution<size_t> dist_row(0, rows - 1);
      size_t row_split = dist_row(rnd);

      for (size_t i = 0; i < rows; ++i)
        for (size_t j = 0; j < cols; ++j)
          child[i][j] = i < row_split ? parent1[i][j] : parent2[i][j];
    } break;

    case CrossoverOp::ColSplit: {
      std::uniform_int_distribution<size_t> dist_col(0, cols - 1);
      size_t col_split = dist_col(rnd);

      for (size_t i = 0; i < rows; ++i)
        for (size_t j = 0; j < cols; ++j)
          child[i][j] = j < col_split ? parent1[i][j] : parent2[i][j];
    } break;

    case CrossoverOp::RowOrColSplit: {
      if (dist_coin(rnd)) {
        std::uniform_int_distribution<size_t> dist_row(0, rows - 1);
        size_t row_split = dist_row(rnd);

        for (size_t i = 0; i < rows; ++i)
          for (size_t j = 0; j < cols; ++j)
            child[i][j] = i < row_split ? parent1[i][j] : parent2[i][j];
      } else {
        std::uniform_int_distribution<size_t> dist_col(0, cols - 1);
        size_t col_split = dist_col(rnd);

        for (size_t i = 0; i < rows; ++i)
          for (size_t j = 0; j < cols; ++j)
            child[i][j] = j < col_split ? parent1[i][j] : parent2[i][j];
      }
    } break;

    case CrossoverOp::PrefRowSplit: {
      size_t row_split = size_t(-1);
      bool flip = false;

      if (dist_coin(rnd)) {
        row_split = size_t((rows - 1) * preference);
      } else {
        row_split = size_t((rows - 1) * (1 - preference));
        flip = true;
      }

      CHECK(row_split < rows);

      for (size_t i = 0; i < row_split; ++i)
        for (size_t j = 0; j < cols; ++j)
          child[i][j] = flip ? parent2[i][j] : parent1[i][j];

      for (size_t i = row_split; i < rows; ++i)
        for (size_t j = 0; j < cols; ++j)
          child[i][j] = flip ? parent1[i][j] : parent2[i][j];
    } break;

    case CrossoverOp::PrefAverage: {
      for (size_t i = 0; i < rows; ++i)
        for (size_t j = 0; j < cols; ++j) {
          auto v1 = parent1[i][j];
          auto v2 = parent2[i][j];

          child[i][j] = v2 + (v1 - v2) * preference;
        }
    } break;

    case CrossoverOp::RowMix: {
      for (size_t i = 0; i < rows; ++i) {
        const auto& parent = dist_parent(rnd) ? parent1 : parent2;
        for (size_t j = 0; j < cols; ++j)
          child[i][j] = parent[i][j];
      }
    } break;

    case CrossoverOp::ColMix: {
      for (size_t j = 0; j < cols; ++j) {
        const auto& parent = dist_parent(rnd) ? parent1 : parent2;
        for (size_t i = 0; i < rows; ++i)
          child[i][j] = parent[i][j];
      }
    } break;

    case CrossoverOp::RowOrColMix: {
      if (dist_coin(rnd)) {
        for (size_t i = 0; i < rows; ++i) {
          const auto& parent = dist_parent(rnd) ? parent1 : parent2;
          for (size_t j = 0; j < cols; ++j)
            child[i][j] = parent[i][j];
        }
      } else {
        for (size_t j = 0; j < cols; ++j) {
          const auto& parent = dist_parent(rnd) ? parent1 : parent2;
          for (size_t i = 0; i < rows; ++i)
            child[i][j] = parent[i][j];
        }
      }
    } break;

    case CrossoverOp::Quadrants: {
      uniform_int_distribution<size_t> dist_row(0, rows - 1);
      uniform_int_distribution<size_t> dist_col(0, cols - 1);

      size_t row_split = dist_row(rnd);
      size_t col_split = dist_col(rnd);

      const auto& parent_q1 = dist_parent(rnd) ? parent1 : parent2;
      const auto& parent_q2 = dist_parent(rnd) ? parent1 : parent2;
      const auto& parent_q3 = dist_parent(rnd) ? parent1 : parent2;
      const auto& parent_q4 = dist_parent(rnd) ? parent1 : parent2;

      for (size_t i = 0; i < row_split; ++i) {
        for (size_t j = 0; j < col_split; ++j)
          child[i][j] = parent_q1[i][j];

        for (size_t j = col_split; j < cols; ++j)
          child[i][j] = parent_q2[i][j];
      }

      for (size_t i = row_split; i < rows; ++i) {
        for (size_t j = 0; j < col_split; ++j)
          child[i][j] = parent_q3[i][j];

        for (size_t j = col_split; j < cols; ++j)
          child[i][j] = parent_q4[i][j];
      }
    } break;

    case CrossoverOp::BestParent: {
      child = (preference > 0.5f) ? parent1 : parent2;
    } break;

    case CrossoverOp::Randomize:
      ann::randomize(child);
      break;
  }
}

void mutationOperator(ann::Matrix& w, float mutation_std_dev) {
  const size_t rows = w.rows;
  const size_t cols = w.cols;

  std::random_device rd;
  std::default_random_engine rnd(rd());
  std::bernoulli_distribution dist_mutate(g_config.mutation_chance);

  switch (g_config.mutation_operator) {
    case MutationOp::IndividualCells: {
      for (size_t i = 0; i < w.rows; ++i)
        for (size_t j = 0; j < w.cols; ++j)
          if (dist_mutate(rnd))
            ann::mutateValue(w[i][j], rnd, mutation_std_dev);
    } break;

    case MutationOp::AllCells:
      if (dist_mutate(rnd)) {
        for (size_t i = 0; i < rows; ++i)
          for (size_t j = 0; j < cols; ++j)
            ann::mutateValue(w[i][j], rnd, mutation_std_dev);
      }
      break;

    case MutationOp::RowOrCol:
      if (dist_mutate(rnd)) {
        std::bernoulli_distribution dist_coin;
        if (dist_coin(rnd)) {
          std::uniform_int_distribution<size_t> dist_row(0, rows - 1);
          size_t row = dist_row(rnd);
          for (size_t i = 0; i < cols; ++i)
            ann::mutateValue(w[row][i], rnd, mutation_std_dev);
        } else {
          std::uniform_int_distribution<size_t> dist_col(0, cols - 1);
          size_t col = dist_col(rnd);
          for (size_t i = 0; i < rows; ++i)
            ann::mutateValue(w[i][col], rnd, mutation_std_dev);
        }
      }
      break;

    case MutationOp::Row:
      if (dist_mutate(rnd)) {
        std::uniform_int_distribution<size_t> dist_row(0, rows - 1);
        size_t row = dist_row(rnd);
        for (size_t i = 0; i < cols; ++i)
          ann::mutateValue(w[row][i], rnd, mutation_std_dev);
      }
      break;

    case MutationOp::Col:
      if (dist_mutate(rnd)) {
        std::uniform_int_distribution<size_t> dist_col(0, cols - 1);
        size_t col = dist_col(rnd);
        for (size_t i = 0; i < rows; ++i)
          ann::mutateValue(w[i][col], rnd, mutation_std_dev);
      }
      break;

    case MutationOp::RowAndCol:
      if (dist_mutate(rnd)) {
        std::uniform_int_distribution<size_t> dist_row(0, rows - 1);
        std::uniform_int_distribution<size_t> dist_col(0, cols - 1);

        size_t row = dist_row(rnd);
        size_t col = dist_col(rnd);

        for (size_t i = 0; i < cols; ++i)
          ann::mutateValue(w[row][i], rnd, mutation_std_dev);

        for (size_t i = 0; i < rows; ++i)
          ann::mutateValue(w[i][col], rnd, mutation_std_dev);
      }
      break;

    case MutationOp::SubRect:
      if (dist_mutate(rnd)) {
        std::uniform_int_distribution<size_t> dist_row(0, rows - 1);
        std::uniform_int_distribution<size_t> dist_col(0, cols - 1);

        size_t row1 = dist_row(rnd);
        size_t row2 = dist_row(rnd);

        if (row1 > row2)
          swap(row1, row2);

        size_t col1 = dist_col(rnd);
        size_t col2 = dist_col(rnd);

        if (col1 > col2)
          swap(col1, col2);

        for (size_t i = row1; i <= row2; ++i)
          for (size_t j = col1; j <= col2; ++j)
            ann::mutateValue(w[i][j], rnd, mutation_std_dev);
      }
      break;
  }
}

}  // namespace classic
