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

#include "runtime.h"
#include "ann_dynamic.h"
#include "platform_abstraction_layer.h"
#include "logging.h"

namespace core {

constexpr char kDarwinHome[] = ".darwin";

#ifdef NDEBUG
constexpr const char* kBuildString = ("RELEASE_BUILD " __DATE__ ", " __TIME__);
#else
constexpr const char* kBuildString = ("DEBUG_BUILD " __DATE__ ", " __TIME__);
#endif  // NDEBUG

fs::path Runtime::darwin_home_path_;

void Runtime::init(int /*argc*/, char* /*argv*/ [], const char* home_path) {
  CHECK(darwin_home_path_.empty(), "Darwin runtime has already been initialized");

  // generate Darwin home path (and create the directory if it doesn't exist)
  // (the explicit home_path is used for setting up the testing environment)
  const auto home = (home_path == nullptr) ? pal::userHomePath() : string(home_path);
  darwin_home_path_ = fs::path(home) / kDarwinHome;
  fs::create_directories(darwin_home_path_);

  core::initLogging();
  ann::initAnnLibrary();
}

const fs::path& Runtime::darwinHomePath() {
  CHECK(!darwin_home_path_.empty(), "Darwin runtime must be initialized first");
  return darwin_home_path_;
}

const char* Runtime::buildString() {
  return kBuildString;
}

}  // namespace core
