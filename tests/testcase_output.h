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

#include <core/utils.h>
#include <core/scope_guard.h>
#include <third_party/gtest/gtest.h>

#include <stdarg.h>
#include <stdio.h>
#include <memory>
#include <string>
using namespace std;

namespace core_test {

// A convenient way to generate per-testcase output files and
// comparing the output against the "golden" output
//
// The "golden" version is the checked in file, so the diffs can be
// reviewed with the regular VCS tools. If the new output is confirmed
// to be correct (normally after changing the test case itself), the
// output file should be commited together with the rest of the changes.
//
// NOTE: currently Git is the only VCS supported
//
class TestCaseOutput {
 public:
  TestCaseOutput() {
    auto test_info = ::testing::UnitTest::GetInstance()->current_test_info();
    filename_ = format("%s/%s_%s.out",
                       TEST_OUTPUT_PATH,
                       normalizeName(test_info->test_case_name()).c_str(),
                       normalizeName(test_info->name()).c_str());
    file_ = ::fopen(filename_.get(), "wt");
    CHECK(file_ != nullptr);
  }

  ~TestCaseOutput() {
    CHECK(file_ != nullptr);
    ::fclose(file_);

    checkForDiffs();
  }

  // no copy/move semantics
  TestCaseOutput(const TestCaseOutput&) = delete;
  TestCaseOutput& operator=(const TestCaseOutput&) = delete;

  operator FILE*() const {
    CHECK(file_ != nullptr);
    return file_;
  }

 private:
  static string normalizeName(const string& name) {
    string result = name;
    for (char& c : result) {
      c = ((c == '/' || c == '\\') ? '.' : c);
    }
    return result;
  }

  unique_ptr<char[]> format(const char* format, ...) {
    va_list argptr = {};

    // get the result size
    va_start(argptr, format);
    int size = vsnprintf(nullptr, 0, format, argptr) + 1;
    CHECK(size > 0);
    va_end(argptr);

    // produce the formatted result buffer
    va_start(argptr, format);
    auto result = make_unique<char[]>(size);
    CHECK(vsnprintf(result.get(), size, format, argptr) == size - 1);
    va_end(argptr);

    return result;
  }

  void checkForDiffs() {
    auto test_info = ::testing::UnitTest::GetInstance()->current_test_info();

    // check for diffs (assumes the source tree is versioned using Git)
    auto git_cmd =
        format("git -C \"%s\" diff -- \"%s\"", TEST_OUTPUT_PATH, filename_.get());

#ifdef DARWIN_COMPILER_MSVC
    auto git_output = ::_popen(git_cmd.get(), "rt");
#else
    auto git_output = ::popen(git_cmd.get(), "r");
#endif  // DARWIN_COMPILER_MSVC

    SCOPE_EXIT {
#ifdef DARWIN_COMPILER_MSVC
      ::_pclose(git_output);
#else
      ::pclose(git_output);
#endif  // DARWIN_COMPILER_MSVC
    };

    // do we have any differences?
    if (::fgetc(git_output) != EOF) {
      ADD_FAILURE_AT(test_info->file(), test_info->line())
          << "Output does not match the golden version";
    }
  }

 private:
  FILE* file_ = nullptr;
  unique_ptr<char[]> filename_;
};

}  // namespace core_test
