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

#include "perf_window.h"
#include "pubsub_relay.h"
#include "ui_perf_window.h"

#include <QGraphicsLayout>

#include <QAreaSeries>
#include <QChart>
#include <QValueAxis>
using namespace QtCharts;

PerfWindow::PerfWindow(QWidget* parent) : QFrame(parent), ui(new Ui::PerfWindow) {
  ui->setupUi(this);

  // setup chart
  top_stage_series_->setColor(kTopSeriesColor);
  top_stage_series_->setName("Total");

  auto area_series = new QAreaSeries(top_stage_series_);
  area_series->setColor(kAreaSeriesColor);
  area_series->setOpacity(0.4);
  area_series->setPen(QPen(Qt::black));

  auto chart = new QChart;
  chart->addSeries(area_series);
  chart->createDefaultAxes();
  chart->setAnimationOptions(QChart::NoAnimation);
  chart->legend()->hide();
  chart->setMargins(QMargins(4, 0, 4, 0));
  chart->layout()->setMinimumSize(0, 0);
  chart->setMinimumSize(200, 100);
  top_stage_series_->setParent(chart);

  // scale chart
  auto x_axis = static_cast<QValueAxis*>(chart->axisX());
  auto y_axis = static_cast<QValueAxis*>(chart->axisY());
  x_axis->setRange(0, kInitialGenerations);
  x_axis->setMinorTickCount(3);
  x_axis->setTickCount(kXTicks + 1);
  x_axis->setLabelFormat("%d");
  y_axis->setRange(0, 1);
  y_axis->setMinorTickCount(1);
  y_axis->setTickCount(kYTicks + 1);

  ui->chart_view->setChart(chart);
  ui->chart_view->setRenderHint(QPainter::Antialiasing);
  ui->chart_view->setRubberBand(QChartView::HorizontalRubberBand);

  connect(
      PubSubRelay::instance(), &PubSubRelay::sigTopStage, this, &PerfWindow::updateChart);
}

PerfWindow::~PerfWindow() {
  delete ui;
}

void PerfWindow::updateChart(const darwin::EvolutionStage& stage) {
  // don't track stages marked as paused or canceled
  const auto annotations = stage.annotations();
  using Annotation = darwin::EvolutionStage::Annotation;
  bool paused = (annotations & Annotation::Paused) != 0;
  bool canceled = (annotations & Annotation::Canceled) != 0;
  bool generation_stage = (annotations & Annotation::Generation) != 0;
  if (paused || canceled || !generation_stage)
    return;

  auto chart = ui->chart_view->chart();
  auto x_axis = static_cast<QValueAxis*>(chart->axisX());
  auto y_axis = static_cast<QValueAxis*>(chart->axisY());

  // update series
  auto elapsed = stage.elapsed();
  top_stage_series_->append(stage_index_, elapsed);

  // auto scale chart
  int rounded_generation = (stage_index_ + (kXTicks - 1)) / kXTicks * kXTicks;
  x_axis->setRange(0, fmax(rounded_generation, x_axis->max()));
  y_axis->setRange(0, fmax(elapsed * kAutoRangeScale, y_axis->max()));

  ++stage_index_;
}
