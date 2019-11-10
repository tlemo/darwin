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

#include "box2d_sandbox_dialog.h"
#include "ui_box2d_sandbox_dialog.h"

namespace physics_ui {

Box2dSandboxDialog::Box2dSandboxDialog(int default_generation, int default_max_steps)
    : QDialog(nullptr,
              Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
      ui(new Ui::Box2dSandboxDialog) {
  ui->setupUi(this);

  ui->generation->setValue(default_generation);
  ui->max_steps->setValue(default_max_steps);

  ui->generation->setFocus();
}

Box2dSandboxDialog::~Box2dSandboxDialog() {
  delete ui;
}

int Box2dSandboxDialog::generation() const {
  return ui->generation->value();
}

int Box2dSandboxDialog::maxSteps() const {
  return ui->max_steps->value();
}

}  // namespace physics_ui
