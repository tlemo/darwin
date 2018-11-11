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

#pragma once

#include "exception.h"

#include <third_party/json/json.h>
using nlohmann::json;

#include <assert.h>
#include <vector>
using namespace std;

namespace core {

//! A 2d array slice (zero-based index, known size)
//! \note ArrayView doesn't take ownership over the array memory it points to
template <class T>
class ArrayView {
 public:
  //! Constructs an array view
  ArrayView(T* array, size_t size) : array_(array), size_(size) {}

  //! Indexed element access
  T& operator[](size_t index) const {
    assert(index < size_);
    return array_[index];
  }

  //! The count of elements in the array view
  size_t size() const { return size_; }

 private:
  T* const array_;
  const size_t size_;
};

//! A dynamically sized 2D matrix
//!
//! \todo stricter encapsulation?
//! \todo CHECK instead of assert?
//! \todo rows/cols/values invariants
//!
template <class T>
struct Matrix {
  //! Constructs an empty matrix (zero rows/columns)
  Matrix() = default;

  //! Constructs a matrix with the specified number of rows and columns
  Matrix(size_t rows, size_t cols) : rows(rows), cols(cols) {
    assert(rows > 0);
    assert(cols > 0);
    values.resize(rows * cols);
  }

  //! Indexed access to a row in the matrix
  ArrayView<T> operator[](size_t row) {
    assert(row < rows);
    return { &values[row * cols], cols };
  }

  //! Indexed access to a row in the matrix
  ArrayView<const T> operator[](size_t row) const {
    assert(row < rows);
    return { &values[row * cols], cols };
  }

  //! Returns true if the matrix is empty
  bool empty() const { return values.empty(); }

  //! Conversion to json
  friend void to_json(json& json_obj, const Matrix& m) {
    json_obj["rows"] = m.rows;
    json_obj["cols"] = m.cols;
    json_obj["values"] = m.values;
  }

  //! Conversion from json
  friend void from_json(const json& json_obj, Matrix& m) {
    m.rows = json_obj.at("rows");
    m.cols = json_obj.at("cols");
    m.values = json_obj.at("values").get<vector<T>>();

    if ((m.rows == 0) != (m.cols == 0))
      throw core::Exception("Failed to deserialize matrix");

    if (m.rows * m.cols != m.values.size())
      throw core::Exception("Failed to deserialize matrix");
  }

  size_t rows = 0;
  size_t cols = 0;
  vector<T> values;
};

}  // namespace core
