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

#include <functional>
#include <mutex>
#include <vector>
using namespace std;

namespace core {

//! A type-safe, non-buffered & synchronous publisher-subscriber channel
//! 
//! \todo Generalize this as signals?
//! \todo Provide a subscription scope guard?
//! 
template <class T>
class PubSub {
  using Subscriber = std::function<void(const T&)>;

 public:
  //! Add a subscriber callback
  //! \returns A subscription index, used with unsubscribe()
  int subscribe(const Subscriber& subscriber) {
    unique_lock<mutex> guard(lock_);

    // try to reuse an empty slot first
    for (int i = 0; i < subscribers_.size(); ++i)
      if (!subscribers_[i]) {
        subscribers_[i] = subscriber;
        return i;
      }

    subscribers_.push_back(subscriber);
    return int(subscribers_.size() - 1);
  }

  //! Removes a subscriber
  //! \param subscriber_index - the index returned from subscribe()
  void unsubscribe(int subscriber_index) {
    unique_lock<mutex> guard(lock_);
    CHECK(subscriber_index >= 0 && subscriber_index < subscribers_.size());
    CHECK(subscribers_[subscriber_index]);
    subscribers_[subscriber_index] = nullptr;
  }

  //! Publish a new value
  //! 
  //! Directly calls all the current subscribers (so if any of the subscriber
  //! callbacks blocks, this call will block as well)
  //! 
  void publish(const T& value) const {
    vector<Subscriber> subscribers_snapshot;

    {
      unique_lock<mutex> guard(lock_);
      subscribers_snapshot = subscribers_;
    }

    for (const auto& subscriber : subscribers_snapshot) {
      if (subscriber)
        subscriber(value);
    }
  }

 private:
  vector<Subscriber> subscribers_;
  mutable mutex lock_;
};

}  // namespace core
