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

#include "start_evolution_dialog.h"
#include "ui_start_evolution_dialog.h"

#include <QPushButton>

StartEvolutionDialog::StartEvolutionDialog(QWidget* parent)
    : QDialog(parent,
              Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
      ui(new Ui::StartEvolutionDialog) {
  ui->setupUi(this);

  ui->button_box->button(QDialogButtonBox::Ok)->setText("Start");

  ui->properties->addPropertiesSection("Evolution properties", &evolution_config_);
  ui->properties->autoSizeColumns();
}

StartEvolutionDialog::~StartEvolutionDialog() {
  delete ui;
}
