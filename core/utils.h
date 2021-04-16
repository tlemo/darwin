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

#include <memory>
#include <utility>

// compiler, platform identifing macros
// (see https://abseil.io/docs/cpp/platforms/macros)

#if defined(_MSC_VER)
#define DARWIN_COMPILER_MSVC
#elif defined(__GNUC__)
#define DARWIN_COMPILER_GCC
#elif defined(__clang__)
#define DARWIN_COMPILER_CLANG
#else
#error Unsupported compiler
#endif

#if defined(_WIN32)
#define DARWIN_OS_WINDOWS
#elif defined(__linux__)
#define DARWIN_OS_LINUX
#elif defined(__APPLE__)
#define DARWIN_OS_APPLE
#elif defined(__EMSCRIPTEN__)
#define DARWIN_OS_WASM
#else
#error Unsupported OS
#endif

namespace core {

[[noreturn]] void __checkFailed(const char* expr, int line, const char* source);
[[noreturn]] void __checkFailed(const char* expr,
                                int line,
                                const char* source,
                                const char* message,
                                ...);

//! A runtime check, present in all build flavors
#define CHECK(x, ...)                                               \
  do {                                                              \
    if (!(x))                                                       \
      core::__checkFailed(#x, __LINE__, __FILE__, ##__VA_ARGS__); \
  } while (false)

[[noreturn]] void __fatal(const char* message, ...);

//! Unconditional fast-fail (present in all build flavors)
#define FATAL(msg, ...) core::__fatal("\nFATAL: " msg "\n\n", ##__VA_ARGS__)

//! Classes derived from this are not copyable or movable
//! 
//! \warning Technically, a derived class could provide the copy or move operations,
//!   case in which they would defeat the purpose of this mixin base class. Don't do it.
//! 
class NonCopyable {
 public:
  NonCopyable() = default;

  // no copy/move semantics
  NonCopyable(const NonCopyable&) = delete;
  NonCopyable& operator=(const NonCopyable&) = delete;
};

}  // namespace core
