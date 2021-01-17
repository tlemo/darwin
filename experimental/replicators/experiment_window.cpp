
#include "experiment_window.h"
#include "ui_experiment_window.h"
#include "phenotype_widget.h"

namespace experimental::replicators {

ExperimentWindow::ExperimentWindow(QWidget* parent, SpeciesFactory* factory)
    : QFrame(parent), ui(new Ui::ExperimentWindow), factory_(factory) {
  ui->setupUi(this);

  // testing...
  layout_ = new QGridLayout(this);
  for (int row = 0; row < 2; ++row) {
    for (int col = 0; col < 2; ++col) {
      auto content = new PhenotypeWidget(this);
      layout_->addWidget(content, row, col);
    }
  }
}

ExperimentWindow::~ExperimentWindow() {
  delete ui;
}

}  // namespace experimental::replicators
