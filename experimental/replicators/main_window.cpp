
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
  enableExperimentActions(false);
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
      enableExperimentActions(true);
    } catch (const core::Exception& e) {
      QMessageBox::warning(this, "Failed to create the experiment", e.what());
    }
  }
}

void MainWindow::closeTab(int index) {
  Q_ASSERT(index >= 0);
  delete ui->tabs->widget(index);
  if (ui->tabs->count() == 0) {
    enableExperimentActions(false);
  }
}

void MainWindow::enableExperimentActions(bool enabled) {
  ui->action_refresh_candidates->setEnabled(enabled);
  ui->action_close_tab->setEnabled(enabled);
  ui->action_animate_phenotypes->setEnabled(enabled);
  ui->action_debug_render->setEnabled(enabled);
}

ExperimentWindow* MainWindow::currentExperimentWindow() const {
  return dynamic_cast<ExperimentWindow*>(ui->tabs->currentWidget());
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

void MainWindow::on_action_refresh_candidates_triggered() {
  if (auto experiment_window = currentExperimentWindow()) {
    experiment_window->refreshCandidates();
  }
}

void MainWindow::on_action_animate_phenotypes_toggled(bool checked) {
  if (auto experiment_window = currentExperimentWindow()) {
    experiment_window->setAnimated(checked);
  }
}

void MainWindow::on_action_debug_render_toggled(bool checked) {
  if (auto experiment_window = currentExperimentWindow()) {
    experiment_window->setDebugRender(checked);
  }
}

void MainWindow::on_tabs_currentChanged(int /*index*/) {
  if (auto experiment_window = currentExperimentWindow()) {
    ui->action_debug_render->setChecked(experiment_window->debugRender());
    ui->action_animate_phenotypes->setChecked(experiment_window->animated());
  }
}

}  // namespace experimental::replicators
