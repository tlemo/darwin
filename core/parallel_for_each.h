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

#include "utils.h"
#include "scope_guard.h"
#include "thread_pool.h"

namespace pp {

// per-thread state used to catch accidental nesting of parallel-for-loops
extern thread_local bool g_inside_parallel_for;

//! Iterates over an array, with support for parallel execution
//! 
//! pp::for_each() offers an easy way to parallelize the processing of the elements in an
//! array (the array must support `size()` and `operator[]`):
//!
//! ```cpp
//! vector<int> array;
//!
//! // parallel-for-loop
//! pp::for_each(array, [](int index, int& value) {
//!   ... read and/or mutate the value ...
//! });
//! ```
//!
//! \note pp::for_each() loops can't be nested
//!
//! \warning Iterations will likely happen on different threads,
//!   so the access to any shared state must be properly synchronized:
//!   ```cpp
//!   int counter = 0;
//!   pp::for_each(array, [&](int index, int& value) {
//!     ...
//!     // BUG: counter is captured by reference and shared between multiple
//!     // iterations, so the increment below introduces a data race
//!     ++counter;
//!   });
//!   ```
//!
template <class T, class Body>
void for_each(T& array, const Body& loop_body) {
  CHECK(!g_inside_parallel_for);

  auto thread_pool = ParallelForSupport::threadPool();
  CHECK(thread_pool != nullptr);

  if (array.size() == 0)
    return;

  // this is roughly the number of shards per worker thread
  //
  // here, a shard is a set of array indexes. higher granularity means
  // smaller individual shards and results in better load balancing,
  //  but also higher work queue synchronization overhead
  //
  // TODO: investigate auto-tuning solutions instead of the hardcoded value
  //
  constexpr int kShardsGranularity = 100;

  const int size = int(array.size());
  const int shards_count = thread_pool->threadsCount() * kShardsGranularity;
  const int shard_size = size / shards_count;
  const int remainder = size % shards_count;

  // create a batch for all the shards
  auto batch = make_unique<WorkBatch>();

  int index = 0;
  for (int i = 0; i < shards_count && index < size; ++i) {
    int actual_shard_size = i < remainder ? (shard_size + 1) : shard_size;
    CHECK(actual_shard_size > 0);
    batch->pushWork([&, beginIndex = index, endIndex = index + actual_shard_size] {
      g_inside_parallel_for = true;
      SCOPE_EXIT { g_inside_parallel_for = false; };

      CHECK(beginIndex < endIndex);

      for (int i = beginIndex; i < endIndex; ++i) {
        loop_body(i, array[i]);
      }
    });

    index += actual_shard_size;
  }
  CHECK(index == size);

  // push work and wait for completition
  thread_pool->processBatch(std::move(batch));
}

}  // namespace pp
