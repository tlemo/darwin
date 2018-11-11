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

#include "utils.h"
#include "logging.h"

#include <stdarg.h>
#include <stdlib.h>
using namespace std;

namespace core {

void __fatal(const char* message, ...) {
  va_list arg_list = {};
  va_start(arg_list, message);
  core::vFatalMessage(message, arg_list);
  va_end(arg_list);

  std::abort();
}

void __checkFailed(const char* expr, int line, const char* source) {
  __fatal("\nCHECK failed [%s] at %s:%d\n\n", expr, source, line);
}

void __checkFailed(const char* expr,
                   int line,
                   const char* source,
                   const char* message,
                   ...) {
  core::fatalMessage("\nFATAL: ");

  va_list arg_list = {};
  va_start(arg_list, message);
  core::vFatalMessage(message, arg_list);
  va_end(arg_list);

  core::fatalMessage("\n");

  __checkFailed(expr, line, source);
}

}  // namespace core
