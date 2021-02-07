
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
  if (sample_set_) {
    sampleGeneration();
  } else {
    newGeneration(factory_->primordialGenotype());
  }
}

void ExperimentWindow::newGeneration(unique_ptr<Genotype> parent) {
  deletePhenotypeWidgets();

  parent_ = std::move(parent);
  population_.clear();
  for (int i = 0; i < kDefaultPopulationSize; ++i) {
    auto clone = parent_->clone();
    clone->mutate();
    population_.push_back(std::move(clone));
  }

  createPhenotypeWidgets();
}

void ExperimentWindow::sampleGeneration() {
  deletePhenotypeWidgets();

  parent_.reset();
  population_ = factory_->samples();
  if (population_.empty()) {
    throw core::Exception("Empty sample set");
  }

  createPhenotypeWidgets();
}

void ExperimentWindow::pickGenotype(size_t index) {
  newGeneration(population_[index]->clone());
}

void ExperimentWindow::deletePhenotypeWidgets() {
  for (const auto widget : layout_->children()) {
    delete widget;
  }
}

void ExperimentWindow::createPhenotypeWidgets() {
  for (size_t i = 0; i < population_.size(); ++i) {
    const auto col = i % kColumns;
    const auto row = i / kColumns;
    const auto widget = new PhenotypeWidget(this, population_[i]->grow());
    connect(widget, &PhenotypeWidget::sigClicked, [=] { pickGenotype(i); });
    layout_->addWidget(widget, row, col);
  }
}

}  // namespace experimental::replicators
