
#pragma once

#include <core/utils.h>

#include <functional>
#include <map>
using namespace std;

// CONSIDER: recurring actions?
class Script {
  using Action = std::function<void(float)>;

 public:
  Script() = default;

  void record(float timestamp, const Action& action) {
    CHECK(!play_);
    actions_.emplace(timestamp, action);
  }

  void start() {
    current_action_ = actions_.begin();
    play_ = true;
  }

  void play(float t) {
    CHECK(play_);
    while (current_action_ != actions_.end() && current_action_->first <= t) {
      current_action_->second(t);
      ++current_action_;
    }
  }

  void clear() {
    actions_.clear();
    current_action_ = actions_.end();
    play_ = false;
  }

 private:
  multimap<float, Action> actions_;
  multimap<float, Action>::const_iterator current_action_;
  bool play_ = false;
};
