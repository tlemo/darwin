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
#include "pubsub.h"

#include <stdarg.h>
#include <string>
#include <utility>
using namespace std;

// CONSIDER: log::verbose(), log::info(), log::exception(), log::fatal() ?

namespace core {

//! Console output messages
inline PubSub<string>* consoleOutput() {
  static PubSub<string>* output = new PubSub<string>;
  return output;
}

//! Logs messsages for severe error conditions
//! (normally just before terminating the program)
void fatalMessage(const char* format, ...);

void vFatalMessage(const char* format, va_list arg_list);

void log(const string& message);

//! Outputs a formatted log message
template <class... ARGS>
void log(const char* format_string, ARGS&&... args) {
  log(core::format(format_string, std::forward<ARGS>(args)...));
}

void initLogging();

}  // namespace core
