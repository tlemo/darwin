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

#include "console_buffer.h"
#include "main_window.h"
#include "settings.h"

#include <core/utils.h>
#include <core/darwin.h>
#include <core/evolution.h>
#include <core/runtime.h>
#include <core/logging.h>
#include <registry/registry.h>
#include <registry/registry_ui.h>

#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  Q_INIT_RESOURCE(resources);

  // console output is buffered
  // (so we can display early messages in the output window)
  core::consoleOutput()->subscribe(
      [](const string& message) { ConsoleBuffer::instance()->addMessage(message); });

  core::Runtime::init(argc, argv);

  g_settings.init();

  darwin::Evolution::init();

  // initialize the registry of populations and domains
  registry::init();
  registry_ui::init();

  MainWindow main_window;
  main_window.show();

  return app.exec();
}
