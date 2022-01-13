// Copyright The Darwin Neuroevolution Framework Authors.
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

#include "random.h"

#include <mutex>

namespace core {

std::random_device::result_type randomSeed() {
#ifdef DARWIN_DETERMINISTIC_RANDOM_SEED
  static std::mutex lock;
  static std::default_random_engine rnd(12345);
  static std::uniform_int_distribution<std::random_device::result_type> dist;
  std::unique_lock guard(lock);
  return dist(rnd);
#else
  return std::random_device{}();
#endif
}

}  // namespace core
