
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
  const auto dlg_title = sample_set ? "New sample set" : "New experiment";
  auto dlg = new NewExperimentDialog(this, dlg_title);
  dlg->setAttribute(Qt::WA_DeleteOnClose);
  connect(dlg, &NewExperimentDialog::sigNewExperiment, [=](QString species_name) {
    this->openTab(species_name, sample_set);
  });
  dlg->open();
}

void MainWindow::openTab(const QString& species_name, bool sample_set) {
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

void MainWindow::closeTab(int index) {
  Q_ASSERT(index >= 0);
  deactivateCurrentExperiment();
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

void MainWindow::deactivateCurrentExperiment() {
  if (current_experiment_window_) {
    current_experiment_window_->setActive(false);
    current_experiment_window_ = nullptr;
  }
}

void MainWindow::dockWindow(const char* name,
                            QFrame* window,
                            Qt::DockWidgetAreas allowed_areas,
                            Qt::DockWidgetArea area) {
  auto dock = new QDockWidget;
  dock->setAllowedAreas(allowed_areas);
  dock->setWindowTitle(window->windowTitle());
  dock->setObjectName(name);
  dock->setMinimumSize(100, 75);
  dock->setWidget(window);
  addDockWidget(area, dock);

  ui->menu_window->addAction(dock->toggleViewAction());
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
  if (current_experiment_window_) {
    current_experiment_window_->refreshCandidates();
  }
}

void MainWindow::on_action_animate_phenotypes_toggled(bool checked) {
  if (current_experiment_window_) {
    current_experiment_window_->setAnimated(checked);
  }
}

void MainWindow::on_action_debug_render_toggled(bool checked) {
  if (current_experiment_window_) {
    current_experiment_window_->setDebugRender(checked);
  }
}

void MainWindow::on_tabs_currentChanged(int /*index*/) {
  deactivateCurrentExperiment();
  current_experiment_window_ = dynamic_cast<ExperimentWindow*>(ui->tabs->currentWidget());
  if (current_experiment_window_) {
    ui->action_debug_render->setChecked(current_experiment_window_->debugRender());
    ui->action_animate_phenotypes->setChecked(current_experiment_window_->animated());
    current_experiment_window_->setActive(true);
  }
}

}  // namespace experimental::replicators
