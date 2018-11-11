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

#include "ann_dynamic.h"
#include "logging.h"
#include "platform_abstraction_layer.h"

#include <assert.h>
#include <immintrin.h>

namespace ann {

EvaluateLayer evaluateLayer = nullptr;

// AVX2 optimized fully connected layer evaluation
static void evaluateLayer_avx(const vector<float>& in,
                              vector<float>& out,
                              const Matrix& w) {
  assert(in.size() + 1 == w.rows);
  assert(out.size() == w.cols);

  const size_t cols = w.cols;
  const size_t bias_index = w.rows - 1;

  for (size_t j = 0; j < cols / 8; ++j) {
    __m256 r = _mm256_loadu_ps(&w[bias_index][j * 8]);

    for (size_t i = 0; i < bias_index; ++i) {
      __m256 a = _mm256_broadcast_ss(&in[i]);
      __m256 b = _mm256_loadu_ps(&w[i][j * 8]);
      r = _mm256_fmadd_ps(a, b, r);
    }

    _mm256_storeu_ps(&out[j * 8], r);
  }

  const size_t mod = cols % 8;
  if (mod != 0) {
    __m256i mask = _mm256_set_epi32((mod > 7) ? -1 : 0,
                                    (mod > 6) ? -1 : 0,
                                    (mod > 5) ? -1 : 0,
                                    (mod > 4) ? -1 : 0,
                                    (mod > 3) ? -1 : 0,
                                    (mod > 2) ? -1 : 0,
                                    (mod > 1) ? -1 : 0,
                                    (mod > 0) ? -1 : 0);

    size_t j = cols & ~7;
    __m256 r = _mm256_maskload_ps(&w[bias_index][j], mask);

    for (size_t i = 0; i < bias_index; ++i) {
      __m256 a = _mm256_broadcast_ss(&in[i]);
      __m256 b = _mm256_maskload_ps(&w[i][j], mask);
      r = _mm256_fmadd_ps(a, b, r);
    }

    _mm256_maskstore_ps(&out[j], mask, r);
  }
}

static void evaluateLayer_cpu(const vector<float>& in,
                              vector<float>& out,
                              const Matrix& w) {
  assert(in.size() + 1 == w.rows);
  assert(out.size() == w.cols);

  const size_t bias_index = w.rows - 1;
  for (size_t i = 0; i < w.cols; ++i) {
    float value = w[bias_index][i];
    for (size_t j = 0; j < bias_index; ++j)
      value += in[j] * w[j][i];
    out[i] = value;
  }
}

void initAnnLibrary() {
  if (pal::detectAvx2()) {
    core::log("ANN library: Using AVX2 optimized code\n");
    evaluateLayer = &evaluateLayer_avx;
  } else {
    core::log("ANN library: AVX2 not detected\n");
    evaluateLayer = &evaluateLayer_cpu;
  }
}

}  // namespace ann
