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

#include "domain_ui.h"
#include "sandbox_window.h"

#include <core/darwin.h>
#include <core/logging.h>

#include <memory>
using namespace std;

namespace drone_track_ui {

void init() {
  darwin::registry()->domains_ui.add<Factory>("drone_track");
}

QWidget* Factory::newSandboxWindow() {
  auto sandbox_window = make_unique<SandboxWindow>();
  if (!sandbox_window->setup()) {
    core::log("Failed to setup the new sandbox window\n\n");
    sandbox_window.reset();
  }
  return sandbox_window.release();
}

}  // namespace drone_track_ui
