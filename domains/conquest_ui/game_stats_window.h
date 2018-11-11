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

#include <domains/conquest/game.h>

#include <QFrame>

#include <QLineSeries>
using namespace QtCharts;

namespace conquest_ui {

namespace Ui {
class GameStatsWindow;
}

class GameStatsWindow : public QFrame {
  Q_OBJECT

  const QColor kBlueColor{ 128, 128, 255 };
  const QColor kRedColor{ 255, 128, 128 };

 public:
  explicit GameStatsWindow(QWidget* parent = nullptr);
  ~GameStatsWindow();

 public:
  void updateCharts(const conquest::Game* game);
  void resetCharts();

 private:
  void setupCharts();

 private:
  Ui::GameStatsWindow* ui;

  int last_chart_step_ = -1;

  QLineSeries* blue_units_series_ = new QLineSeries;
  QLineSeries* red_units_series_ = new QLineSeries;

  QLineSeries* total_nodes_series_ = new QLineSeries;
  QLineSeries* red_nodes_series_ = new QLineSeries;

  QLineSeries* blue_orders_series_ = new QLineSeries;
  QLineSeries* red_orders_series_ = new QLineSeries;
};

}  // namespace conquest_ui
