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

#include "fitness_window.h"
#include "pubsub_relay.h"
#include "settings.h"
#include "ui_fitness_window.h"

#include <QChart>
#include <QGraphicsLayout>
#include <QLineSeries>
#include <QPen>
#include <QSplineSeries>
#include <QValueAxis>
using namespace QtCharts;

#include <random>
using namespace std;

static QLineSeries* createSeries(const QPen& pen) {
  QLineSeries* series =
      g_settings.spline_fitness_series ? new QSplineSeries : new QLineSeries;
  series->setPen(pen);
  return series;
}

FitnessWindow::FitnessWindow(QWidget* parent)
    : QFrame(parent), ui(new Ui::FitnessWindow) {
  ui->setupUi(this);

  // setup chart
  best_fitness_series_ = createSeries(QPen(Qt::blue, 2.0));
  median_fitness_series_ = createSeries(QPen(Qt::gray));
  worst_fitness_series_ = createSeries(QPen(Qt::red));

  auto chart = new QChart;
  chart->addSeries(best_fitness_series_);
  chart->addSeries(median_fitness_series_);
  chart->addSeries(worst_fitness_series_);
  chart->createDefaultAxes();
  chart->setAnimationOptions(QChart::NoAnimation);
  chart->legend()->hide();
  chart->setMargins(QMargins(4, 4, 4, 4));
  chart->layout()->setMinimumSize(0, 0);
  chart->setMinimumSize(200, 100);

  // scale chart
  auto x_axis = static_cast<QValueAxis*>(chart->axisX());
  auto y_axis = static_cast<QValueAxis*>(chart->axisY());
  x_axis->setRange(0, kInitialGenerations);
  x_axis->setMinorTickCount(1);
  x_axis->setTickCount(kXTicks + 1);
  x_axis->setLabelFormat("%d");
  y_axis->setRange(0, 1);
  y_axis->setMinorTickCount(1);
  y_axis->setTickCount(kYTicks + 1);

  ui->chart_view->setChart(chart);
  ui->chart_view->setRenderHint(QPainter::Antialiasing);
  ui->chart_view->setRubberBand(QChartView::HorizontalRubberBand);

  connect(PubSubRelay::instance(),
          &PubSubRelay::sigGenerationSummary,
          this,
          &FitnessWindow::updateChart);
}

FitnessWindow::~FitnessWindow() {
  delete ui;
}

void FitnessWindow::updateChart(const darwin::GenerationSummary& summary) {
  auto chart = ui->chart_view->chart();
  auto x_axis = static_cast<QValueAxis*>(chart->axisX());
  auto y_axis = static_cast<QValueAxis*>(chart->axisY());
  
  float current_max = 0;
  float current_min = 0;

  // update main series
  best_fitness_series_->append(summary.generation, summary.best_fitness);
  median_fitness_series_->append(summary.generation, summary.median_fitness);
  worst_fitness_series_->append(summary.generation, summary.worst_fitness);
  
  current_max = max(current_max, summary.best_fitness);
  current_min = min(current_min, summary.worst_fitness);

  // do we have extra fitness series?
  if (summary.calibration_fitness) {
    for (auto property : summary.calibration_fitness->properties()) {
      auto [series_it, inserted] =
          extra_fitness_series_.insert({ property->name(), nullptr });

      // new series
      if (inserted) {
        uniform_int_distribution<int> dist_H(0, 359);
        auto color = QColor::fromHsv(dist_H(rnd), 255, 255);
        auto pen = QPen(color, 1, Qt::DashLine);
        auto series = createSeries(pen);
        chart->addSeries(series);
        CHECK(series->attachAxis(x_axis));
        CHECK(series->attachAxis(y_axis));
        series_it->second = series;
      }

      float value = property->nativeValue<float>();
      series_it->second->append(summary.generation, value);
      current_max = max(current_max, value);
      current_min = min(current_min, value);
    }
  }

  // auto scale horizontal axis
  int rounded_generation = (summary.generation + (kXTicks - 1)) / kXTicks * kXTicks;
  x_axis->setRange(0, fmax(rounded_generation, x_axis->max()));

  // auto scale vertical axis
  float scaled_max = current_max * kFitnessAutoRange;
  float scaled_min = current_min * kFitnessAutoRange;
  y_axis->setRange(fmin(scaled_min, y_axis->min()),
                   fmax(scaled_max, y_axis->max()));
}
