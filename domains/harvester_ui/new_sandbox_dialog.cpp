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
#include <domains/harvester/world_map.h>

using namespace harvester;

namespace harvester_ui {

NewSandboxDialog::NewSandboxDialog(QWidget* parent)
    : QDialog(parent,
              Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
      ui(new Ui::NewSandboxDialog) {
  ui->setupUi(this);

  auto snapshot = darwin::evolution()->snapshot();
  ui->generation->setValue(snapshot.generation - 1);

  ui->world_width->setValue(g_config.map_width);
  ui->world_height->setValue(g_config.map_height);
  ui->initial_health->setValue(g_config.initial_health);

  ui->generation->setFocus();
}

NewSandboxDialog::~NewSandboxDialog() {
  delete ui;
}

int NewSandboxDialog::generation() const {
  return ui->generation->value();
}

int NewSandboxDialog::worldWidth() const {
  return ui->world_width->value();
}

int NewSandboxDialog::worldHeight() const {
  return ui->world_height->value();
}

int NewSandboxDialog::initialHealth() const {
  return ui->initial_health->value();
}

}  // namespace harvester_ui
