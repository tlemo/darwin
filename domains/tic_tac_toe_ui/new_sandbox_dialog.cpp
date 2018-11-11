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

#include "new_sandbox_dialog.h"
#include "ui_new_sandbox_dialog.h"

#include <core/darwin.h>
#include <core/evolution.h>

namespace tic_tac_toe_ui {

NewSandboxDialog::NewSandboxDialog(QWidget* parent)
    : QDialog(parent,
              Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
      ui(new Ui::NewSandboxDialog) {
  ui->setupUi(this);

  auto snapshot = darwin::evolution()->snapshot();
  ui->x_player_generation->setValue(snapshot.generation - 1);
  ui->o_player_generation->setValue(snapshot.generation - 1);

  ui->x_player_type->setFocus();
  updateUi();
}

NewSandboxDialog::~NewSandboxDialog() {
  delete ui;
}

QString NewSandboxDialog::xPlayerType() const {
  return ui->x_player_type->currentText();
}

QString NewSandboxDialog::oPlayerType() const {
  return ui->o_player_type->currentText();
}

int NewSandboxDialog::xPlayerGeneration() const {
  return ui->x_player_generation->value();
}

int NewSandboxDialog::oPlayerGeneration() const {
  return ui->o_player_generation->value();
}

void NewSandboxDialog::updateUi() {
  ui->x_player_generation->setEnabled(ui->x_player_type->currentText() == kAiPlayer);
  ui->o_player_generation->setEnabled(ui->o_player_type->currentText() == kAiPlayer);
}

}  // namespace tic_tac_toe_ui
