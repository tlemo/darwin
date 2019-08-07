// Copyright 2019 The Darwin Neuroevolution Framework Authors.
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

namespace core {

struct DynamicInitializer {
  explicit DynamicInitializer(void (*pfn)()) { (*pfn)(); }
};

#define _GLOBAL_INITIALIZER_HELPER(id)                                              \
  void dynamicInitialierBody_##id();                                                \
  core::DynamicInitializer g_dynamic_initializer_##id(&dynamicInitialierBody_##id); \
  void dynamicInitialierBody_##id()

#define _GLOBAL_INITIALIZER_DEF(id) _GLOBAL_INITIALIZER_HELPER(id)

// A helper for defining global (dynamic) initializers. For example:
//
//  GLOBAL_INITIALIZER {
//    printf("Initializing...\n");
//    registerType("Foo");
//  }
//
// This macro is intended to be used at namespace scope and it's subject to the same
// initialization gotchas as the initialization of global objects (initialization order
// across translation units is not specified)
//
// NOTE: GLOBAL_INITIALIZER should NOT be used in headers
//
#define GLOBAL_INITIALIZER _GLOBAL_INITIALIZER_DEF(__COUNTER__)

}  // namespace core
