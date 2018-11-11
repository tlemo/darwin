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

namespace pong_ui {

NewSandboxDialog::NewSandboxDialog(QWidget* parent)
    : QDialog(parent,
              Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
      ui(new Ui::NewSandboxDialog) {
  ui->setupUi(this);

  auto snapshot = darwin::evolution()->snapshot();
  ui->generation_left_player->setValue(snapshot.generation - 1);
  ui->generation_right_player->setValue(snapshot.generation - 1);

  ui->left_player_type->setFocus();
  updateUi();
}

NewSandboxDialog::~NewSandboxDialog() {
  delete ui;
}

QString NewSandboxDialog::leftPlayerType() const {
  return ui->left_player_type->currentText();
}

QString NewSandboxDialog::rightPlayerType() const {
  return ui->right_player_type->currentText();
}

int NewSandboxDialog::leftPlayerGeneration() const {
  return ui->generation_left_player->value();
}

int NewSandboxDialog::rightPlayerGeneration() const {
  return ui->generation_right_player->value();
}

void NewSandboxDialog::updateUi() {
  ui->generation_left_player->setEnabled(ui->left_player_type->currentText() ==
                                         kAiPlayer);
  ui->generation_right_player->setEnabled(ui->right_player_type->currentText() ==
                                          kAiPlayer);
}

void NewSandboxDialog::on_left_player_type_currentIndexChanged(const QString&) {
  updateUi();
}

void NewSandboxDialog::on_right_player_type_currentIndexChanged(const QString&) {
  updateUi();
}

}  // namespace pong_ui
