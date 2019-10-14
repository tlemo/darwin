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

#include <core/utils.h>

#include <functional>
#include <map>
using namespace std;

namespace physics {

//! Basic action scripting
//! 
//! Arbrirary actions are recorded together with a timestamp,
//! then played in sequence (ordered by the reference timestamp, which is assumed to
//! be strictly advancing)
//! 
//! \todo Recurrent actions?
//! 
class Script {
  using Action = std::function<void(float)>;

  static constexpr float kStartTimestamp = -1;
  static constexpr float kInvalidTimestamp = -2;

 public:
  void record(float t, const Action& action) {
    CHECK(timestamp_ == kInvalidTimestamp);
    actions_.emplace(t, action);
  }

  void start() {
    current_action_ = actions_.begin();
    timestamp_ = kStartTimestamp;
  }

  void play(float t) {
    CHECK(timestamp_ != kInvalidTimestamp);
    CHECK(t > timestamp_);
    while (current_action_ != actions_.end() && current_action_->first <= t) {
      current_action_->second(t);
      ++current_action_;
    }
    timestamp_ = t;
  }

  void clear() {
    actions_.clear();
    current_action_ = actions_.end();
    timestamp_ = kInvalidTimestamp;
  }

 private:
  multimap<float, Action> actions_;
  multimap<float, Action>::const_iterator current_action_;
  float timestamp_ = kInvalidTimestamp;
};

}  // namespace physics
