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

#pragma once

#include <filesystem>
namespace fs = std::filesystem;

namespace core {

//! Interface to the Darwin runtime environment
class Runtime {
 public:
  //! Initialize the Darwin runtime
  static void init(int argc, char* argv[], const char* home_path = nullptr);

  //! The Darwin home path (containing configuration and log files)
  static const fs::path& darwinHomePath();

  //! A signature string describing the current build
  static const char* buildString();

 private:
  static fs::path darwin_home_path_;
};

}  // namespace core
