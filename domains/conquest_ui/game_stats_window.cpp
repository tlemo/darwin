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

#include "game_stats_window.h"
#include "ui_game_stats_window.h"

#include <QAreaSeries>
#include <QChart>
#include <QLineSeries>
#include <QValueAxis>
using namespace QtCharts;

namespace conquest_ui {

GameStatsWindow::GameStatsWindow(QWidget* parent)
    : QFrame(parent), ui(new Ui::GameStatsWindow) {
  ui->setupUi(this);
  setupCharts();
}

GameStatsWindow::~GameStatsWindow() {
  delete ui;
}

static void scaleChart(QChartView* chart_view, float max_x, float max_y) {
  auto chart = chart_view->chart();
  auto x_axis = static_cast<QValueAxis*>(chart->axisX());
  auto y_axis = static_cast<QValueAxis*>(chart->axisY());
  x_axis->setRange(0, fmax(max_x, x_axis->max()));
  y_axis->setRange(0, fmax(max_y, y_axis->max()));
  x_axis->setLabelFormat("%d");
}

void GameStatsWindow::updateCharts(const conquest::Game* game) {
  auto step = game->currentStep();
  if (step == last_chart_step_)
    return;
  last_chart_step_ = step;

  auto stats = game->computeStats();

  // units chart
  blue_units_series_->append(step, stats.blue_units);
  red_units_series_->append(step, stats.red_units);
  scaleChart(ui->units_chart, step, stats.blue_units + stats.red_units);

  // nodes chart
  auto total_nodes = stats.blue_nodes + stats.red_nodes;
  total_nodes_series_->append(step, total_nodes);
  red_nodes_series_->append(step, stats.red_nodes);
  scaleChart(ui->nodes_chart, step, total_nodes);

  // orders chart
  blue_orders_series_->append(step, stats.blue_orders);
  red_orders_series_->append(step, stats.red_orders);
  scaleChart(ui->orders_chart, step, stats.blue_orders + stats.red_orders);
}

static void createChart(QChartView* chart_view,
                        const char* title,
                        QAbstractSeries* s1,
                        QAbstractSeries* s2) {
  auto chart = new QChart;
  chart->addSeries(s1);
  chart->addSeries(s2);
  chart->createDefaultAxes();
  chart->setTitle(title);
  chart->setAnimationOptions(QChart::NoAnimation);
  chart->legend()->hide();

  chart_view->setChart(chart);
  chart_view->setRenderHint(QPainter::Antialiasing);

  scaleChart(chart_view, 100, 10);
}

void GameStatsWindow::setupCharts() {
  // units
  blue_units_series_->setColor(Qt::blue);
  red_units_series_->setColor(Qt::red);
  createChart(ui->units_chart, "Units", blue_units_series_, red_units_series_);

  // orders
  blue_orders_series_->setColor(Qt::blue);
  red_orders_series_->setColor(Qt::red);
  createChart(ui->orders_chart, "Orders", blue_orders_series_, red_orders_series_);

  // nodes (stacked chart)
  auto red_area_series = new QAreaSeries(red_nodes_series_);
  red_area_series->setColor(kRedColor);
  red_area_series->setPen(QPen(Qt::white));

  auto blue_area_series = new QAreaSeries(total_nodes_series_, red_nodes_series_);
  blue_area_series->setColor(kBlueColor);
  blue_area_series->setPen(QPen(Qt::white));

  createChart(ui->nodes_chart, "Nodes", blue_area_series, red_area_series);
}

void GameStatsWindow::resetCharts() {
  last_chart_step_ = -1;

  // units
  blue_units_series_->clear();
  red_units_series_->clear();
  ui->units_chart->chart()->axisX()->setRange(0, 500);
  ui->units_chart->chart()->axisY()->setRange(0, 5);

  // nodes
  total_nodes_series_->clear();
  red_nodes_series_->clear();
  ui->nodes_chart->chart()->axisX()->setRange(0, 500);
  ui->nodes_chart->chart()->axisY()->setRange(0, 8);

  // orders
  blue_orders_series_->clear();
  red_orders_series_->clear();
  ui->orders_chart->chart()->axisX()->setRange(0, 500);
  ui->orders_chart->chart()->axisY()->setRange(0, 10);
}

}  // namespace conquest_ui
