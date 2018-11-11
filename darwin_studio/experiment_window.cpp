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

#include "experiment_window.h"
#include "pubsub_relay.h"
#include "ui_experiment_window.h"

#include <core/darwin.h>
#include <core/format.h>

ExperimentWindow::ExperimentWindow(shared_ptr<darwin::Experiment> experiment)
    : QFrame(nullptr), ui(new Ui::ExperimentWindow), experiment_(experiment) {
  ui->setupUi(this);

  newExperiment();
}

ExperimentWindow::~ExperimentWindow() {
  delete ui;
}

void ExperimentWindow::startingExperiment() {
  ui->properties->setEnabled(false);
  ui->properties->clearSelection();

  if (ui->properties->isModified())
    experiment_->newVariation();
}

void ExperimentWindow::newExperiment() {
  // set window title
  string title = "Experiment";
  if (experiment_->name().has_value())
    title += core::format(" (%s)", experiment_->name().value());
  setWindowTitle(QString::fromStdString(title));

  ui->properties->clear();

  // experiment section
  auto setup_section = ui->properties->addSection("Setup");
  for (auto property : experiment_->setup()->properties()) {
    auto property_item = setup_section->addProperty(property->name());
    property_item->setValue(property->value());
    property_item->setTextColor(1, Qt::darkGray);
  }

  // mutable sections
  addProperties("Core", experiment_->coreConfig());
  addProperties("Domain", experiment_->domainConfig());
  addProperties("Population", experiment_->populationConfig());

  ui->properties->autoSizeColumns();
}

void ExperimentWindow::addProperties(const string& name, core::PropertySet* config) {
  if (!config->properties().empty())
    ui->properties->addPropertiesSection(name, config);
}
