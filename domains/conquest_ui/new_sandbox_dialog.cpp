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

namespace conquest_ui {

NewSandboxDialog::NewSandboxDialog(QWidget* parent)
    : QDialog(parent,
              Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
      ui(new Ui::NewSandboxDialog) {
  ui->setupUi(this);

  ui->blue_player_generation->setEnabled(ui->blue_player_type->currentText() ==
                                         kAiPlayer);
  ui->red_player_generation->setEnabled(ui->red_player_type->currentText() == kAiPlayer);

  auto snapshot = darwin::evolution()->snapshot();
  ui->blue_player_generation->setValue(snapshot.generation - 1);
  ui->red_player_generation->setValue(snapshot.generation - 1);

  ui->blue_player_type->setFocus();
}

NewSandboxDialog::~NewSandboxDialog() {
  delete ui;
}

QString NewSandboxDialog::bluePlayerType() const {
  return ui->blue_player_type->currentText();
}

QString NewSandboxDialog::redPlayerType() const {
  return ui->red_player_type->currentText();
}

int NewSandboxDialog::bluePlayerGeneration() const {
  return ui->blue_player_generation->value();
}

int NewSandboxDialog::redPlayerGeneration() const {
  return ui->red_player_generation->value();
}

void NewSandboxDialog::on_blue_player_type_currentIndexChanged(const QString& type) {
  ui->blue_player_generation->setEnabled(type == kAiPlayer);
}

void NewSandboxDialog::on_red_player_type_currentIndexChanged(const QString& type) {
  ui->red_player_generation->setEnabled(type == kAiPlayer);
}

}  // namespace conquest_ui
