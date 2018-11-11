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

#include <core/darwin.h>
#include <core/evolution.h>
#include <core/thread_pool.h>
#include <registry/registry.h>

#include <third_party/gtest/gtest.h>

#include <memory>
using namespace std;

class DummyProgressMonitor final : public darwin::ProgressMonitor {
  void beginStage(const string&, size_t, uint32_t) final {}
  void finishStage(const string&) final {}
  void reportProgress(size_t) final {}
};

int main(int argc, char* argv[]) {
  // Darwin initialization
  //
  // NOTE: this must be done before InitGoogleTest() in order
  //  to allow parameterized tests query the environment
  //
  darwin::init(0, nullptr, TEST_TEMP_PATH);
  registry::init();

  auto monitor = make_unique<DummyProgressMonitor>();
  darwin::ProgressManager::registerMonitor(monitor.get());

  pp::ParallelForSupport::init(nullptr);

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
