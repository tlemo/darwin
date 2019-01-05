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

#include "new_sandbox_dialog.h"
#include "ui_new_sandbox_dialog.h"

#include <core/darwin.h>
#include <core/evolution.h>
#include <domains/cart_pole/cart_pole.h>

namespace cart_pole_ui {

NewSandboxDialog::NewSandboxDialog(QWidget* parent)
    : QDialog(parent,
              Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
      ui(new Ui::NewSandboxDialog) {
  ui->setupUi(this);

  auto snapshot = darwin::evolution()->snapshot();
  auto cart_pole = dynamic_cast<const cart_pole::CartPole*>(snapshot.domain);
  CHECK(cart_pole != nullptr);
  ui->generation->setValue(snapshot.generation - 1);
  ui->max_steps->setValue(cart_pole->config().max_steps);

  ui->generation->setFocus();
}

NewSandboxDialog::~NewSandboxDialog() {
  delete ui;
}

int NewSandboxDialog::generation() const {
  return ui->generation->value();
}

int NewSandboxDialog::maxSteps() const {
  return ui->max_steps->value();
}

}  // namespace cart_pole_ui
