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
#include <core/properties.h>

#include <QFrame>

#include <memory>
using namespace std;

namespace Ui {
class ExperimentWindow;
}

class ExperimentWindow : public QFrame {
  Q_OBJECT

 public:
  explicit ExperimentWindow(shared_ptr<darwin::Experiment> experiment);
  ~ExperimentWindow();

  void onBeginRun();
  void onAbortRun();

 private:
  void newExperiment();
  void addProperties(const string& name, core::PropertySet* config);

 private:
  Ui::ExperimentWindow* ui;

  shared_ptr<darwin::Experiment> experiment_;
};
