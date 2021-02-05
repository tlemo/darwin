// Copyright The Darwin Neuroevolution Framework Authors.
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

#include "tests.h"
#include "replicators.h"

#include <core/utils.h>

#include <QMessageBox>

#include <exception>

namespace experimental::replicators::tests {

void runTests() {
  // the following try/catch will not handle other types of failure
  // (ex. segfault or abort)
  try {
    for (const auto& [name, factory] : *registry()) {
      factory->runTests();
    }
  } catch (std::exception& e) {
    QMessageBox::warning(nullptr, "Test failure", e.what());
    FATAL("Test case failure");
  } catch (...) {
    QMessageBox::warning(nullptr, "Test failure", "Unknown exception");
    FATAL("Test case failure");
  }
}

}  // namespace experimental::replicators::tests
