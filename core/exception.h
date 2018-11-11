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

#include "format.h"

#include <exception>
#include <string>
#include <utility>
using namespace std;

namespace core {

//! The base for exception types in the Darwin framework
//! 
//! \note This type is move-only
//!
class Exception : public std::exception {
 public:
  //! Constructs an exception object, optionally formatting the message
  //! \sa core::format()
  template <class... ARGS>
  explicit Exception(const char* message, ARGS&&... args)
      : message_(core::format(message, std::forward<ARGS>(args)...)) {}

  // move only
  Exception(Exception&&) noexcept = default;
  Exception& operator=(Exception&&) noexcept = default;

  //! Returns the exception message
  const char* what() const noexcept override { return message_.c_str(); }

 private:
  string message_;
};

}  // namespace core
