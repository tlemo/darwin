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

namespace Ui {
class NewExperimentDialog;
}

class NewExperimentDialog : public QDialog {
  Q_OBJECT

 public:
  explicit NewExperimentDialog(QWidget* parent,
                               const darwin::Universe* universe,
                               const darwin::Experiment* parent_experiment);

  ~NewExperimentDialog();

  void done(int result) override;

  QString experimentName() const;
  QString populationName() const;
  QString domainName() const;
  int populationSize() const;
  darwin::ComplexityHint populationHint() const;
  darwin::ComplexityHint domainHint() const;

 private slots:
  void on_experiment_name_textChanged(const QString&);

 private:
  Ui::NewExperimentDialog* ui;

  const darwin::Universe* universe_ = nullptr;
};
