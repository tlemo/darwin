// Copyright 2019 The Darwin Neuroevolution Framework Authors.
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

#include <QDialog>

namespace physics_ui {

namespace Ui {
class Box2dSandboxDialog;
}

class Box2dSandboxDialog : public QDialog {
  Q_OBJECT

 public:
  explicit Box2dSandboxDialog(int default_generation, int default_max_steps);
  ~Box2dSandboxDialog();

  int generation() const;
  int maxSteps() const;

 private:
  Ui::Box2dSandboxDialog* ui = nullptr;
};

}  // namespace physics_ui
