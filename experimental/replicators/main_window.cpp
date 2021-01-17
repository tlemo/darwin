
#include "main_window.h"
#include "ui_main_window.h"
#include "phenotype_widget.h"

namespace experimental::replicators {

MainWindow::MainWindow() : QMainWindow(nullptr), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  layout_ = new QGridLayout(ui->central_widget);

  // testing...
  for (int row = 0; row < 2; ++row) {
    for (int col = 0; col < 2; ++col) {
      auto content = new PhenotypeWidget(ui->central_widget);
      content->setBorderSize(4);
      content->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
      layout_->addWidget(content, row, col);
    }
  }
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::on_action_new_experiment_triggered() {
  // TODO
}

}  // namespace experimental::replicators
