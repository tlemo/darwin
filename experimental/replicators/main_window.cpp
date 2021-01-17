
#include "main_window.h"
#include "ui_main_window.h"
#include "new_experiment_dialog.h"
#include "experiment_window.h"
#include "replicators.h"

namespace experimental::replicators {

MainWindow::MainWindow() : QMainWindow(nullptr), ui(new Ui::MainWindow) {
  ui->setupUi(this);
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::on_action_new_experiment_triggered() {
  NewExperimentDialog dlg(this, "New experiment");
  if (dlg.exec() == QDialog::Accepted) {
    const auto& scene_name = dlg.speciesName();
    const auto factory = registry()->find(scene_name.toStdString());
    CHECK(factory != nullptr);
    auto experiment_window = make_unique<ExperimentWindow>(this, factory);
    auto new_tab_index = ui->tabs->addTab(experiment_window.release(), scene_name);
    ui->tabs->setCurrentIndex(new_tab_index);
  }
}

void MainWindow::on_action_new_sample_set_triggered() {
  NewExperimentDialog dlg(this, "New sample set");
  if (dlg.exec() == QDialog::Accepted) {
    // TODO
  }
}

void MainWindow::on_tabs_tabCloseRequested(int index) {
  delete ui->tabs->widget(index);
}

}  // namespace experimental::replicators
