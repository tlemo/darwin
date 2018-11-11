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

#include "new_experiment_dialog.h"
#include "ui_new_experiment_dialog.h"

#include <core/stringify.h>

#include <string>
using namespace std;

NewExperimentDialog::NewExperimentDialog(QWidget* parent,
                                         const darwin::Universe* universe,
                                         const darwin::Experiment* parent_experiment)
    : QDialog(parent,
              Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
      ui(new Ui::NewExperimentDialog),
      universe_(universe) {
  ui->setupUi(this);
  ui->duplicate_name->hide();

  // branching from an existing experiment?
  if (parent_experiment != nullptr) {
    const auto& setup = parent_experiment->setup();

    ui->population_size->setValue(setup->population_size);

    ui->domains->addItem(QString::fromStdString(setup->domain_name));
    ui->populations->addItem(QString::fromStdString(setup->population_name));

    ui->domain_hint->addItem(QString::fromStdString(core::toString(setup->domain_hint)));
    ui->population_hint->addItem(
        QString::fromStdString(core::toString(setup->population_hint)));

    ui->domains->setEnabled(false);
    ui->domain_hint->setEnabled(false);
    ui->populations->setEnabled(false);
    ui->population_hint->setEnabled(false);

    setWindowTitle("Branch from current experiment");
    ui->population_size->setFocus();
  } else {
    auto registry = darwin::registry();

    constexpr int kDefaultPopulationSize = 5000;
    ui->population_size->setValue(kDefaultPopulationSize);

    for (const auto& factory : registry->domains)
      ui->domains->addItem(factory.first.c_str());

    for (const auto& factory : registry->populations)
      ui->populations->addItem(factory.first.c_str());

    for (const auto& hint : core::knownValues<darwin::ComplexityHint>()) {
      ui->population_hint->addItem(hint.c_str());
      ui->domain_hint->addItem(hint.c_str());
    }

    // TODO: use experiment defaults?
    ui->domain_hint->setCurrentText("balanced");
    ui->population_hint->setCurrentText("balanced");

    setWindowTitle("Create new experiment");
    ui->domains->setFocus();
  }
}

NewExperimentDialog::~NewExperimentDialog() {
  delete ui;
}

void NewExperimentDialog::done(int result) {
  if (result == QDialog::Accepted) {
    const string name = experimentName().toStdString();
    if (!name.empty() && universe_->findExperiment(name)) {
      ui->duplicate_name->show();
      return;
    }
  }

  QDialog::done(result);
}

void NewExperimentDialog::on_experiment_name_textChanged(const QString&) {
  ui->duplicate_name->hide();
}

QString NewExperimentDialog::experimentName() const {
  return ui->experiment_name->text();
}

QString NewExperimentDialog::populationName() const {
  return ui->populations->currentText();
}

QString NewExperimentDialog::domainName() const {
  return ui->domains->currentText();
}

int NewExperimentDialog::populationSize() const {
  return ui->population_size->value();
}

darwin::ComplexityHint NewExperimentDialog::populationHint() const {
  return core::fromString<darwin::ComplexityHint>(
      ui->population_hint->currentText().toStdString());
}

darwin::ComplexityHint NewExperimentDialog::domainHint() const {
  return core::fromString<darwin::ComplexityHint>(
      ui->domain_hint->currentText().toStdString());
}
