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

#include <map>
#include <random>
#include <string>
using namespace std;

namespace Ui {
class FitnessWindow;
}

class FitnessWindow : public QFrame {
  Q_OBJECT

  static constexpr int kInitialGenerations = 20;
  static constexpr int kXTicks = 10;
  static constexpr int kYTicks = 4;
  static constexpr float kFitnessAutoRange = 1.1f;

 public:
  explicit FitnessWindow(QWidget* parent = nullptr);
  ~FitnessWindow();

 private:
  void updateChart(const darwin::GenerationSummary& summary);

 private:
  Ui::FitnessWindow* ui;

  QLineSeries* best_fitness_series_ = nullptr;
  QLineSeries* median_fitness_series_ = nullptr;
  QLineSeries* worst_fitness_series_ = nullptr;

  map<string, QLineSeries*> extra_fitness_series_;

  // use a fixed seed to get consistent colors
  default_random_engine rnd{ 2 };
};
