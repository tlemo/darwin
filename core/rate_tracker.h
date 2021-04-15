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

#pragma once

#include <assert.h>
#include <chrono>
using namespace std;

namespace core {

//! A simple event-per-second rate tracker (ex. FPS)
class RateTracker {
  using Clock = std::chrono::steady_clock;

  static constexpr int kMaxTrackedUpdates = 30;
  static constexpr int kMinAggregatedUpdates = 5;

 public:
  RateTracker() { reset(); }

  void reset() {
    next_update_ = 0;
    tracked_updates_ = 0;
  }

  void update() {
    auto time_stamp = Clock::now();
    assert(next_update_ < kMaxTrackedUpdates);
    update_times_[next_update_++] = time_stamp;
    if (next_update_ == kMaxTrackedUpdates)
      next_update_ = 0;
    if (tracked_updates_ < kMaxTrackedUpdates)
      ++tracked_updates_;
  }

  double currentRate() const {
    if (tracked_updates_ < kMinAggregatedUpdates)
      return 0;

    int last_update = (kMaxTrackedUpdates + next_update_ - 1) % kMaxTrackedUpdates;
    int oldest_update =
        (kMaxTrackedUpdates + next_update_ - tracked_updates_) % kMaxTrackedUpdates;

    chrono::duration<double> delta =
        update_times_[last_update] - update_times_[oldest_update];
    double seconds = delta.count();
    assert(seconds > 0);
    return seconds > 0 ? tracked_updates_ / seconds : 0;
  }

 private:
  Clock::time_point update_times_[kMaxTrackedUpdates];
  int next_update_ = 0;
  int tracked_updates_ = 0;
};

}  // namespace core
