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

#include "output_window.h"
#include "console_buffer.h"
#include "pubsub_relay.h"
#include "ui_output_window.h"

OutputWindow::OutputWindow(QWidget* parent) : QFrame(parent), ui(new Ui::OutputWindow) {
  ui->setupUi(this);

  connect(PubSubRelay::instance(),
          &PubSubRelay::sigConsoleOutput,
          this,
          &OutputWindow::consoleOutput);

  // flush buffered messages
  ConsoleBuffer::instance()->flushAndResume();
}

OutputWindow::~OutputWindow() {
  delete ui;
}

void OutputWindow::consoleOutput(const QString& output) {
  ui->output->moveCursor(QTextCursor::End);
  ui->output->insertPlainText(output);
  ui->output->ensureCursorVisible();
}
