// Copyright 2020 The Darwin Neuroevolution Framework Authors.
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

#include <core/utils.h>
#include <core/format.h>
#include <third_party/gtest/gtest.h>

#include <stdlib.h>

namespace python_tests {

// TODO: find a way to avoid mutating the environment
//
TEST(PythonBindingsTest, RunPythonTests) {
  // the PYTHON_CMD, if set, must point to the appropriate Python interpreter
  const char* python_cmd = std::getenv("PYTHON_CMD");

  // if PYTHON_CMD is not set, skip running Python tests
  if (python_cmd == nullptr) {
    GTEST_SKIP();
  }

  // we need to update PYTHONPATH to point to the darwin extension
  CHECK(setenv("PYTHONPATH", "../../bindings/python", true) == 0);

  // force a custom root location
  // (this will also be used to store test universe files)
  CHECK(setenv("DARWIN_HOME_PATH", TEST_TEMP_PATH, true) == 0);

  // invoke Python unit tests and check the exit code
  int exit_code = std::system(
      core::format("%s -m unittest discover %s", python_cmd, TEST_PYTHON_SUITE).c_str());
  EXPECT_EQ(exit_code, 0);
}

}  // namespace python_tests
