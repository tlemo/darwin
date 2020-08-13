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

#include <core/universe.h>

#include <third_party/gtest/gtest.h>

#include <filesystem>
namespace fs = std::filesystem;

class DarwinTestEnvironment : public testing::Environment {
 public:
  static string universePath() {
    const auto universe_path = fs::path(TEST_TEMP_PATH) / "test_universe.darwin";
    return universe_path.string();
  }

 private:
  void SetUp() override {
    auto universe_path = universePath();
    fs::remove(universe_path);
    darwin::Universe::create(universe_path);
  }

  void TearDown() override {}
};
