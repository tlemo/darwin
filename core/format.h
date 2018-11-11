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

#include <stdarg.h>
#include <stdio.h>
#include <string>
#include <type_traits>
using namespace std;

namespace core {

namespace internal {

inline string formatHelper(const char* format_string, ...) {
  va_list argptr = {};

  // calculate the result string size
  va_start(argptr, format_string);
  int size = vsnprintf(nullptr, 0, format_string, argptr);
  CHECK(size >= 0);
  va_end(argptr);

  // actually format into the result string
  va_start(argptr, format_string);
  string result(size, 0);
  CHECK(vsnprintf(result.data(), size + 1, format_string, argptr) == size);
  va_end(argptr);

  return result;
}

template <class T>
T formatArg(T value) {
  static_assert(is_scalar_v<T>, "Only scalar values and strings can be formatted");
  return value;
}

inline const char* formatArg(const string& s) {
  return s.c_str();
}

}  // namespace internal

//! A minimalistic string formatting built on top of the C-formatting
//! facilities (xprintf formatting)
//! 
//! In addition to the plain printf(), core::format() allows std::string to be passed
//! directly for %s conversion specifiers.
//! 
//! \todo Add runtime check for the match between the format and the value arguments
//! 
template <class... ARGS>
string format(const char* format_string, ARGS&&... args) {
  CHECK(format_string != nullptr);
  return internal::formatHelper(format_string, internal::formatArg(args)...);
}

}  // namespace core
