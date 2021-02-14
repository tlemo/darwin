
#include "main_window.h"
#include "ui_main_window.h"
#include "new_experiment_dialog.h"
#include "experiment_window.h"
#include "replicators.h"

#include <core/exception.h>

#include <QMessageBox>

namespace experimental::replicators {

MainWindow::MainWindow() : QMainWindow(nullptr), ui(new Ui::MainWindow) {
  ui->setupUi(this);
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::newExperimentWindow(bool sample_set) {
  NewExperimentDialog dlg(this, sample_set ? "New sample set" : "New experiment");
  if (dlg.exec() == QDialog::Accepted) {
    const auto species_name = dlg.speciesName();
    const auto factory = registry()->find(species_name.toStdString());
    CHECK(factory != nullptr);

    try {
      const auto tab_title = species_name + (sample_set ? " (Samples)" : "");
      auto experiment_window = make_unique<ExperimentWindow>(this, factory, sample_set);
      auto new_tab_index = ui->tabs->addTab(experiment_window.release(), tab_title);
      ui->tabs->setCurrentIndex(new_tab_index);
      ui->action_refresh_candidates->setEnabled(true);
      ui->action_close_tab->setEnabled(true);
    } catch (const core::Exception& e) {
      QMessageBox::warning(this, "Failed to create the experiment", e.what());
    }
  }
}

void MainWindow::closeTab(int index) {
  delete ui->tabs->widget(index);
  if (ui->tabs->count() == 0) {
    ui->action_refresh_candidates->setEnabled(false);
    ui->action_close_tab->setEnabled(false);
  }
}

void MainWindow::on_action_new_experiment_triggered() {
  newExperimentWindow(false);
}

void MainWindow::on_action_new_sample_set_triggered() {
  newExperimentWindow(true);
}

void MainWindow::on_tabs_tabCloseRequested(int index) {
  closeTab(index);
}

void MainWindow::on_action_close_tab_triggered() {
  Q_ASSERT(ui->tabs->count() > 0);
  closeTab(ui->tabs->currentIndex());
}

}  // namespace experimental::replicators
