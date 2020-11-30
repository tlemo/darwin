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

#include "platform_abstraction_layer.h"
#include "utils.h"
#include "exception.h"

#include <stdlib.h>

#ifdef DARWIN_COMPILER_MSVC
#include <intrin.h>
#endif  // DARWIN_COMPILER_MSVC

#include <filesystem>
namespace fs = std::filesystem;

namespace pal {

static string homePathHelper() {
#ifdef DARWIN_OS_WINDOWS
  const char* user_profile = getenv("USERPROFILE");
  if (user_profile != nullptr)
    return user_profile;

  const char* home_drive = getenv("HOMEDRIVE");
  const char* home_path = getenv("HOMEPATH");
  if (home_drive != nullptr && home_path != nullptr)
    return string(home_drive) + string(home_path);
#endif  // DARWIN_OS_WINDOWS

  const char* home = getenv("HOME");
  if (home != nullptr)
    return home;

  throw core::Exception("Can't locate home directory");
}

string userHomePath() {
  auto home = homePathHelper();
  if (!fs::is_directory(home))
    throw core::Exception("Invalid home path: '%s'", home);
  return home;
}

bool detectAvx2() {
#ifdef DARWIN_COMPILER_MSVC
  int cpu_info[4] = {};
  bool has_avx = false;

  __cpuid(cpu_info, 0);
  const int n_ids = cpu_info[0];

  if (n_ids >= 7) {
    __cpuidex(cpu_info, 7, 0);
    const int ebx_reg = cpu_info[1];
    has_avx = (ebx_reg & (1 << 5)) != 0;
  }

  return has_avx;
#else
  return __builtin_cpu_supports("avx2");
#endif  // DARWIN_COMPILER_MSVC
}

void setenv(const char* name, const char* value) {
#ifdef DARWIN_OS_WINDOWS
  CHECK(::_putenv_s(name, value) == 0);
#else
  CHECK(::setenv(name, value, true) == 0);
#endif  // DARWIN_OS_WINDOWS
}

}  // namespace pal
