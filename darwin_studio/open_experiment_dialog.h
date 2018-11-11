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

#pragma once

#include <core/darwin.h>
#include <core/universe.h>

#include <QDialog>
#include <QTreeWidgetItem>

namespace Ui {
class OpenExperimentDialog;
}

class OpenExperimentDialog : public QDialog {
  Q_OBJECT

 public:
  explicit OpenExperimentDialog(QWidget* parent, const darwin::Universe* universe);
  ~OpenExperimentDialog();

  const darwin::DbExperiment* experiment() const;

 private:
  void populateExperimentsTable();

 private slots:
  void on_experiments_itemActivated(QTreeWidgetItem* item, int column);

 private:
  Ui::OpenExperimentDialog* ui;

  const darwin::Universe* universe_ = nullptr;
};
