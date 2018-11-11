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

#include "logging.h"
#include "utils.h"
#include "format.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <array>
#include <atomic>
using namespace std;

namespace core {

static atomic<FILE*> g_logfile = nullptr;

static bool g_premature_messages = false;

void log(const string& message) {
  auto stream = g_logfile.load();
  if (stream != nullptr) {
    fprintf(stream, "%s", message.c_str());
    fflush(stream);
  } else {
    g_premature_messages = true;
    printf("%s", message.c_str());
  }

  consoleOutput()->publish(message);
}

void initLogging() {
  // build the session log filename: 'session_YYYYMMDD_HHMMSS.log'
  //
  // NOTE: this will need to be revisited:
  //  1. localtime() is likely not thread-safe
  //  2. using second granularity is not a great way to avoid conflicts
  //
  array<char, 128> filename = {};
  time_t timestamp = time(nullptr);
  CHECK(strftime(filename.data(),
                 filename.size(),
                 "session_%Y%m%d_%H%M%S.log",
                 localtime(&timestamp)) > 0);

  // open the session log file
  FILE* logfile = ::fopen(filename.data(), "wt");
  CHECK(logfile != nullptr, "Can't create the log file (%s)", filename.data());
  CHECK(g_logfile.exchange(logfile) == nullptr);

  // technically this check would be a data race, although
  // we should not have any extra threads at this point
  // (this is a best effort warning anyway)
  if (g_premature_messages) {
    log("WARNING: there were early messages logged only to the console\n");
  }
}

void vFatalMessage(const char* format, va_list arg_list) {
  auto stream = g_logfile.load();
  if (stream != nullptr) {
    va_list arg_list_copy = {};
    va_copy(arg_list_copy, arg_list);
    vfprintf(stream, format, arg_list_copy);
    fflush(stream);
    va_end(arg_list_copy);
  }

  vfprintf(stderr, format, arg_list);
}

void fatalMessage(const char* format, ...) {
  va_list arg_list = {};
  va_start(arg_list, format);
  vFatalMessage(format, arg_list);
  va_end(arg_list);
}

}  // namespace core
