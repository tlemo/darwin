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

#include <utility>

namespace core {

// a developer friendly scope guard and macro
// (inspired by Andrei Alexandrescu's C++11 Scope Guard)
class ScopeGuardHelper {
  template <class T>
  class ScopeGuard {
   public:
    explicit ScopeGuard(T closure) : closure_(std::move(closure)) {}

    ~ScopeGuard() { closure_(); }

    // move constructor only
    ScopeGuard(ScopeGuard&&) = default;
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;
    ScopeGuard& operator=(ScopeGuard&&) = delete;

   private:
    T closure_;
  };

 public:
  template <class T>
  ScopeGuard<T> operator<<(T closure) {
    return ScopeGuard<T>(std::move(closure));
  }
};

#define _SG_MACRO_CONCAT2(a, b) a##b
#define _SG_MACRO_CONCAT(a, b) _SG_MACRO_CONCAT2(a, b)
#define _SG_ANONYMOUS(prefix) _SG_MACRO_CONCAT(prefix, __COUNTER__)

#define SCOPE_EXIT auto _SG_ANONYMOUS(_scope_guard_) = core::ScopeGuardHelper() << [&]()

}  // namespace core
