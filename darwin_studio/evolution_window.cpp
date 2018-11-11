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

#include "evolution_window.h"
#include "pubsub_relay.h"
#include "ui_evolution_window.h"

#include <assert.h>

EvolutionWindow::EvolutionWindow(QWidget* parent)
    : QFrame(parent), ui(new Ui::EvolutionWindow) {
  ui->setupUi(this);
  ui->history->setHorizontalHeaderLabels(QStringList({ "Best", "Median", "Worst" }));

  updateUI(darwin::Evolution::EventFlag::All);

  connect(
      PubSubRelay::instance(), &PubSubRelay::sigEvents, this, &EvolutionWindow::updateUI);
}

EvolutionWindow::~EvolutionWindow() {
  delete ui;
}

void EvolutionWindow::updateUI(uint32_t hints) {
  auto snapshot = darwin::evolution()->snapshot();

  ui->generation->setValue(snapshot.generation);

  if ((hints & darwin::Evolution::EventFlag::ProgressUpdate) != 0) {
    ui->progress->setValue(snapshot.stage.progressPercent());
  }

  if ((hints & darwin::Evolution::EventFlag::StateChanged) != 0) {
    ui->stage->setText(snapshot.stage.name().c_str());
    ui->stage->setCursorPosition(0);
  }

  if ((hints & darwin::Evolution::EventFlag::EndGeneration) != 0 && snapshot.trace) {
    const int trace_size = snapshot.trace->size();
    if (trace_size != last_trace_size_) {
      assert(trace_size > 0);
      ui->history->setRowCount(trace_size);
      for (int i = last_trace_size_; i < trace_size; ++i) {
        // row label
        ui->history->setVerticalHeaderItem(
            i, new QTableWidgetItem(QString::asprintf("%d", i)));

        // best/median/worst values
        auto summary = snapshot.trace->generationSummary(i);
        setHistoryValue(i, 0, summary.best_fitness);
        setHistoryValue(i, 1, summary.median_fitness);
        setHistoryValue(i, 2, summary.worst_fitness);
      }
      last_trace_size_ = trace_size;
      ui->history->selectRow(last_trace_size_ - 1);
    }
  }
}

void EvolutionWindow::setHistoryValue(int index, int column, float value) {
  auto item = new QTableWidgetItem(QString::asprintf("%.2f", value));
  ui->history->setItem(index, column, item);
}
