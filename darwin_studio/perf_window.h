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
#include <core/evolution.h>

#include <QFrame>

#include <QLineSeries>
using namespace QtCharts;

namespace Ui {
class PerfWindow;
}

class PerfWindow : public QFrame {
  Q_OBJECT

  static constexpr int kInitialGenerations = 20;
  static constexpr int kXTicks = 10;
  static constexpr int kYTicks = 2;
  static constexpr float kAutoRangeScale = 1.1f;

  const QColor kTopSeriesColor{ 0, 0, 255 };
  const QColor kAreaSeriesColor{ 100, 200, 255 };

 public:
  explicit PerfWindow(QWidget* parent = nullptr);
  ~PerfWindow();

 private:
  void updateChart(const darwin::EvolutionStage& stage);

 private:
  Ui::PerfWindow* ui;

  int stage_index_ = 0;
  QLineSeries* top_stage_series_ = new QLineSeries;
};
