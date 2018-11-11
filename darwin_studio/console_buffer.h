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

#include <core/pubsub.h>

#include <atomic>
#include <mutex>
#include <string>
#include <vector>
using namespace std;

// Buffers console output before reaching the output window
//
// This is a very specialized indirection, to make sure we can capture messages
// even when the output window is not present
//
class ConsoleBuffer {
 public:
  core::PubSub<string> console_output;

 public:
  static ConsoleBuffer* instance() {
    static auto instance = new ConsoleBuffer();
    return instance;
  }

  void addMessage(const string& message) {
    bool publish = true;

    {
      std::unique_lock<mutex> guard(lock_);
      if (buffering_) {
        messages_.push_back(message);
        publish = false;
      }
    }

    if (publish)
      console_output.publish(message);
  }

  // flush all the bufferend messages and switch to passthrough (no buffering)
  void flushAndResume() {
    vector<string> current_messages;

    {
      std::unique_lock<mutex> guard(lock_);
      current_messages.swap(messages_);
      buffering_ = false;
    }

    for (const auto& message : current_messages)
      console_output.publish(message);
  }

 private:
  ConsoleBuffer() = default;

 private:
  mutable mutex lock_;
  vector<string> messages_;
  bool buffering_ = true;
};
