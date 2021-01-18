
#include "experiment_window.h"
#include "ui_experiment_window.h"
#include "phenotype_widget.h"

#include <core/exception.h>

namespace experimental::replicators {

ExperimentWindow::ExperimentWindow(QWidget* parent,
                                   SpeciesFactory* factory,
                                   bool sample_set)
    : QFrame(parent),
      ui(new Ui::ExperimentWindow),
      factory_(factory),
      sample_set_(sample_set) {
  ui->setupUi(this);
  layout_ = new QGridLayout(this);
  resetPopulation();
}

ExperimentWindow::~ExperimentWindow() {
  delete ui;
}

void ExperimentWindow::resetPopulation() {
  // create the population samples
  if (sample_set_) {
    parent_.reset();
    population_ = factory_->samples();
    if (population_.empty()) {
      throw core::Exception("Empty sample set");
    }
  } else {
    parent_ = factory_->primordialGenotype();
    population_.clear();
    for (int i = 0; i < kDefaultPopulationSize; ++i) {
      population_.push_back(parent_->mutate());
    }
  }

  // reset existing widgets
  for (const auto widget : layout_->children()) {
    delete widget;
  }

  // create widgets for the new samples
  for (size_t i = 0; i < population_.size(); ++i) {
    const auto col = i % kColumns;
    const auto row = i / kColumns;
    const auto widget = new PhenotypeWidget(this, population_[i]->grow());
    layout_->addWidget(widget, row, col);
  }
}

}  // namespace experimental::replicators
