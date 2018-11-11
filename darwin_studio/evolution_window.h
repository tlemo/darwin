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

#include <QFrame>
#include <QLabel>

namespace Ui {
class EvolutionWindow;
}

class EvolutionWindow : public QFrame {
  Q_OBJECT

 public:
  explicit EvolutionWindow(QWidget* parent = nullptr);
  ~EvolutionWindow();

 private:
  void updateUI(uint32_t hints);
  void setHistoryValue(int index, int column, float value);

 private:
  Ui::EvolutionWindow* ui;
  int last_trace_size_ = 0;
};
