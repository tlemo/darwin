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

#include "open_experiment_dialog.h"
#include "ui_open_experiment_dialog.h"

#include <core/properties.h>
#include <core/stringify.h>

#include <QPainter>
#include <QPen>
#include <QStyledItemDelegate>

class ExperimentItemDelegate : public QStyledItemDelegate {
 public:
  using QStyledItemDelegate::QStyledItemDelegate;

 private:
  void paint(QPainter* painter,
             const QStyleOptionViewItem& option,
             const QModelIndex& index) const override {
    QStyledItemDelegate::paint(painter, option, index);

    painter->setPen(QPen(Qt::lightGray, 0, Qt::SolidLine));
    painter->drawRect(option.rect);
  }
};

class ExperimentItem : public QTreeWidgetItem {
 public:
  ExperimentItem(const darwin::DbExperiment& db_experiment,
                 const darwin::Universe* universe)
      : db_experiment_(db_experiment), universe_(universe) {
    // load the experiment setup
    darwin::ExperimentSetup setup;
    setup.fromJson(json::parse(db_experiment.setup));

    setColumnText(0, core::toString(db_experiment.name.value_or("")));

    setColumnText(
        1,
        universe_->strftime(db_experiment.last_activity_timestamp, "%Y-%m-%d %H:%M:%S"));

    setColumnText(2, universe_->strftime(db_experiment.timestamp, "%Y-%m-%d %H:%M:%S"));

    setColumnText(3, core::toString(setup.domain_name));
    setColumnText(4, core::toString(setup.population_name));
    setColumnText(5, core::toString(setup.population_size));
    setColumnText(6, core::toString(setup.domain_hint));
    setColumnText(7, core::toString(setup.population_hint));

    // force the desired row height
    setSizeHint(0, QSize(0, 30));
  }

  const darwin::DbExperiment* experiment() const { return &db_experiment_; }

 private:
  void setColumnText(int column, const string& text) {
    setText(column, QString::fromStdString(text));
  }

 private:
  darwin::DbExperiment db_experiment_;
  const darwin::Universe* universe_ = nullptr;
};

OpenExperimentDialog::OpenExperimentDialog(QWidget* parent,
                                           const darwin::Universe* universe)
    : QDialog(parent,
              Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
      ui(new Ui::OpenExperimentDialog),
      universe_(universe) {
  ui->setupUi(this);

  // TODO: async?
  populateExperimentsTable();

  // auto-select top row
  auto top_row = ui->experiments->topLevelItem(0);
  if (top_row != nullptr) {
    ui->experiments->setCurrentItem(top_row);
  }

  ui->experiments->setFocus();
}

OpenExperimentDialog::~OpenExperimentDialog() {
  delete ui;
}

const darwin::DbExperiment* OpenExperimentDialog::experiment() const {
  auto experiment_item = ui->experiments->currentItem();
  return experiment_item != nullptr
             ? dynamic_cast<ExperimentItem*>(experiment_item)->experiment()
             : nullptr;
}

void OpenExperimentDialog::populateExperimentsTable() {
  CHECK(ui->experiments->topLevelItemCount() == 0);

  ui->experiments->setItemDelegate(new ExperimentItemDelegate(ui->experiments));
  ui->experiments->setSortingEnabled(false);

  // TODO: add the comment column
  ui->experiments->setHeaderLabels(QStringList({ "Name",
                                                 "Last activity",
                                                 "Creation Time",
                                                 "Domain",
                                                 "Population",
                                                 "Population Size",
                                                 "Domain Hint",
                                                 "Population Hint" }));

  for (const auto& db_experiment : universe_->experimentsList()) {
    auto experiment_item = new ExperimentItem(db_experiment, universe_);
    ui->experiments->addTopLevelItem(experiment_item);
  }

  // attempt to cleanly auto size the columns
  for (int i = 0; i < ui->experiments->columnCount(); ++i) {
    ui->experiments->resizeColumnToContents(i);
    auto actual_width = ui->experiments->columnWidth(i);
    ui->experiments->setColumnWidth(i, std::max(125, actual_width));
  }

  ui->experiments->sortByColumn(1, Qt::DescendingOrder);
  ui->experiments->setSortingEnabled(true);
}

void OpenExperimentDialog::on_experiments_itemActivated(QTreeWidgetItem*, int) {
  accept();
}
