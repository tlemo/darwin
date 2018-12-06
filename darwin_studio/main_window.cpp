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

#include "main_window.h"
#include "evolution_window.h"
#include "experiment_window.h"
#include "fitness_window.h"
#include "new_experiment_dialog.h"
#include "open_experiment_dialog.h"
#include "output_window.h"
#include "perf_window.h"
#include "pubsub_relay.h"
#include "settings.h"
#include "start_evolution_dialog.h"
#include "ui_main_window.h"

#include <core/logging.h>
#include <core/platform_abstraction_layer.h>

#include <QApplication>
#include <QByteArray>
#include <QFileDialog>
#include <QMessageBox>

#include <memory>
#include <optional>
using namespace std;

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

MainWindow::MainWindow() : QMainWindow(nullptr), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  restoreGeometry();

  // main toolbar menu entry
  ui->menu_window->addAction(ui->main_toolbar->toggleViewAction());

  // status bar
  status_label_ = new QLabel;
  status_label_->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  ui->statusbar->addPermanentWidget(status_label_);

  // configure docking
  setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
  setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
  setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
  setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

  connect(PubSubRelay::instance(),
          &PubSubRelay::sigEvents,
          this,
          &MainWindow::evolutionEvent);

  dockWindow("output_window",
             new OutputWindow,
             Qt::AllDockWidgetAreas,
             Qt::BottomDockWidgetArea,
             true);

  reopenLastUniverse();

  restoreLayout();
  updateUi();
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::dockWindow(const char* name,
                            QFrame* window,
                            Qt::DockWidgetAreas allowed_areas,
                            Qt::DockWidgetArea area,
                            bool permanent) {
  auto dock = new QDockWidget;
  dock->setAllowedAreas(allowed_areas);
  dock->setWindowTitle(window->windowTitle());
  dock->setObjectName(name);
  dock->setMinimumSize(100, 75);
  dock->setWidget(window);
  addDockWidget(area, dock);

  if (!permanent)
    experiment_windows_.push_back(dock);

  ui->menu_window->addAction(dock->toggleViewAction());
}

bool MainWindow::confirmationDialog(const QString& title, const QString& text) {
  QMessageBox confirmation_dialog(this);
  confirmation_dialog.setIcon(QMessageBox::Question);
  confirmation_dialog.setWindowTitle(title);
  confirmation_dialog.setText(text);
  confirmation_dialog.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  confirmation_dialog.setDefaultButton(QMessageBox::No);
  return confirmation_dialog.exec() == QMessageBox::Yes;
}

bool MainWindow::confirmEndOfExperiment() {
  return confirmationDialog(
      "Close the current experiment",
      "Are you sure? This will terminate the current experiment batch.");
}

void MainWindow::closeExperiment() {
  if (g_settings.auto_save_ui_layout)
    saveLayout();

  // clear any existing dock windows
  for (QDockWidget* dock : experiment_windows_) {
    ui->menu_window->removeAction(dock->toggleViewAction());
    delete dock;
  }
  experiment_windows_.clear();

  // clear all sandbox tabs
  while (ui->sandbox_tabs->count() > 0)
    delete ui->sandbox_tabs->widget(0);

  if (experiment_) {
    CHECK(darwin::evolution()->reset());

    active_experiment_ = false;
    batch_total_runs_ = 0;
    batch_current_run_ = 0;
    experiment_.reset();
  }

  restoreLayout();
  updateUi();
}

void MainWindow::createExperimentWindows() {
  CHECK(experiment_windows_.empty());

  // preserve the layout of permanent windows
  if (g_settings.auto_save_ui_layout)
    saveLayout();

  auto experiment_window = make_unique<ExperimentWindow>(experiment_);
  connect(this,
          &MainWindow::sigStartingExperiment,
          experiment_window.get(),
          &ExperimentWindow::startingExperiment);

  dockWindow("experiment_window",
             experiment_window.release(),
             Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea,
             Qt::LeftDockWidgetArea);

  dockWindow("evolution_window",
             new EvolutionWindow,
             Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea,
             Qt::RightDockWidgetArea);

  dockWindow("fitness_window",
             new FitnessWindow,
             Qt::AllDockWidgetAreas,
             Qt::BottomDockWidgetArea);

  dockWindow(
      "perf_window", new PerfWindow, Qt::AllDockWidgetAreas, Qt::TopDockWidgetArea);

  restoreLayout();
}

void MainWindow::reopenLastUniverse() {
  CHECK(!universe_);
  CHECK(!experiment_);
  CHECK(!active_experiment_);
  CHECK(batch_total_runs_ == 0);
  CHECK(batch_current_run_ == 0);

  auto last_universe = g_settings.last_universe;

  if (g_settings.reopen_last_universe && !last_universe.empty()) {
    try {
      core::log("Reopening last universe: '%s'...\n", last_universe);
      universe_ = darwin::Universe::open(last_universe);
    } catch (const exception& e) {
      core::log("Failed to reopen last universe: '%s'\n", e.what());
      g_settings.last_universe = "";
      g_settings.save();
    }
  }
}

void MainWindow::universeSwitched() {
  if (universe_) {
    g_settings.last_universe = universe_->path();
    g_settings.last_directory = fs::path(universe_->path()).parent_path().string();
    g_settings.save();
  }
}

bool MainWindow::resetExperiment() {
  // remember the experiment ID so we can reload it
  auto experiment_id = experiment_->dbExperimentId();

  closeExperiment();

  // reload the experiment
  //
  // TODO: keep the DbExperiment instead of the experiment_id
  //  (last variation might change)
  //
  try {
    auto db_experiment = universe_->loadExperiment(experiment_id);
    CHECK(db_experiment != nullptr);

    experiment_ = make_shared<darwin::Experiment>(db_experiment.get(), universe_.get());

    createExperimentWindows();
  } catch (const std::exception& e) {
    QMessageBox::warning(this, "Can't reset experiment", e.what());
    return false;
  }

  return true;
}

void MainWindow::nextBatchedRun() {
  CHECK(experiment_);
  CHECK(active_experiment_);
  CHECK(batch_total_runs_ > 0);
  CHECK(batch_current_run_ > 0);
  CHECK(batch_current_run_ <= batch_total_runs_);

  if (batch_current_run_ < batch_total_runs_) {
    if (ui->sandbox_tabs->count() > 0) {
      // TODO: revisit this (should we prompt the user instead?)
      core::log("Warning: closing sandbox windows before starting next batched run.\n");
    }

    auto saved_total_runs = batch_total_runs_;
    auto saved_current_run = batch_current_run_;

    CHECK(resetExperiment());

    auto evolution = darwin::evolution();
    active_experiment_ = evolution->newExperiment(experiment_, evolution_config_);
    CHECK(active_experiment_);

    batch_total_runs_ = saved_total_runs;
    batch_current_run_ = saved_current_run + 1;

    core::log("\nStarting run %d/%d\n\n", batch_current_run_, batch_total_runs_);

    evolution->run();
  }

  updateUi();
}

void MainWindow::evolutionEvent(uint32_t event_flags) {
  // end of an evolution run?
  if ((event_flags & darwin::Evolution::EventFlag::EndEvolution) != 0) {
    nextBatchedRun();
  }
  
  // any state change?
  if ((event_flags & darwin::Evolution::EventFlag::StateChanged) != 0) {
    updateUi();
  }
}

void MainWindow::updateUi() {
  auto snapshot = darwin::evolution()->snapshot();

  bool experiment_running = false;
  bool have_domain_ui = false;
  bool runnable = false;

  CHECK(universe_ || !experiment_);
  CHECK(experiment_ || !active_experiment_);

  if (snapshot.experiment && snapshot.experiment->domainUiFactory() != nullptr)
    have_domain_ui = true;

  switch (snapshot.state) {
    case darwin::Evolution::State::Initializing:
    case darwin::Evolution::State::Paused:
      experiment_running = false;
      runnable = true;
      break;

    case darwin::Evolution::State::Stopped:
      experiment_running = false;
      break;

    case darwin::Evolution::State::Running:
    case darwin::Evolution::State::Canceling:
    case darwin::Evolution::State::Pausing:
      experiment_running = true;
      break;

    default:
      FATAL("unexpected state");
  }

  CHECK(!experiment_running || active_experiment_);

  if (active_experiment_) {
    CHECK(batch_total_runs_ > 0);
    CHECK(batch_current_run_ > 0);
    CHECK(batch_current_run_ <= batch_total_runs_);
  } else {
    CHECK(batch_total_runs_ == 0);
    CHECK(batch_current_run_ == 0);
  }

  // enable/disable UI actions
  ui->action_run->setEnabled(experiment_ && runnable);
  ui->action_pause->setEnabled(active_experiment_ && experiment_running);
  ui->action_reset->setEnabled(experiment_ && !experiment_running);
  ui->action_open->setEnabled(!experiment_running);
  ui->action_exit->setEnabled(true);
  ui->action_new->setEnabled(!experiment_running);
  ui->action_new_sandbox->setEnabled(active_experiment_ && have_domain_ui);
  ui->action_new_experiment->setEnabled(universe_ && !experiment_running);
  ui->action_branch_experiment->setEnabled(experiment_ && !experiment_running);
  ui->action_open_experiment->setEnabled(universe_ && !experiment_running);

  // current batch
  auto batch_text = QString::asprintf("Run %d/%d", batch_current_run_, batch_total_runs_);

  // status label
  QString state_text;
  switch (snapshot.state) {
    case darwin::Evolution::State::Initializing:
      state_text = "Initializing";
      break;
    case darwin::Evolution::State::Pausing:
      state_text = "Pausing...";
      break;
    case darwin::Evolution::State::Canceling:
      state_text = "Canceling...";
      break;
    case darwin::Evolution::State::Stopped:
      state_text = QString("Stopped (%1)").arg(batch_text);
      break;
    case darwin::Evolution::State::Paused:
      state_text = QString("Paused (%1)").arg(batch_text);
      break;
    case darwin::Evolution::State::Running:
      state_text = QString("Running (%1)").arg(batch_text);
      break;
    default:
      FATAL("unexpected value");
  }
  status_label_->setText(state_text);

  // main window title
  QString title = "Darwin Studio";
  if (universe_)
    title += QString::asprintf(" (%s)", universe_->path().c_str());
  setWindowTitle(title);
}

void MainWindow::saveGeometry() const {
  auto geometry_data = QMainWindow::saveGeometry();
  g_settings.qt_geometry_data = geometry_data.toBase64().data();
  g_settings.save();
}

void MainWindow::restoreGeometry() {
  if (g_settings.qt_geometry_data.empty())
    return;

  auto geometry_data = QByteArray::fromBase64(g_settings.qt_geometry_data.c_str());
  if (!QMainWindow::restoreGeometry(geometry_data))
    core::log("Failed to restore the window geometry\n");
}

void MainWindow::saveLayout() const {
  auto layout_data = QMainWindow::saveState();

  if (experiment_windows_.empty())
    g_settings.qt_base_layout_data = layout_data.toBase64().data();
  else
    g_settings.qt_experiment_layout_data = layout_data.toBase64().data();

  g_settings.save();
}

void MainWindow::restoreLayout() {
  string encoded_layout_data = experiment_windows_.empty()
                                   ? g_settings.qt_base_layout_data
                                   : g_settings.qt_experiment_layout_data;

  if (!encoded_layout_data.empty()) {
    auto layout_data = QByteArray::fromBase64(encoded_layout_data.c_str());
    if (!QMainWindow::restoreState(layout_data))
      core::log("Failed to restore the windows layout\n");
  }
}

void MainWindow::closeEvent(QCloseEvent* event) {
  if (g_settings.auto_save_ui_layout) {
    saveGeometry();
    saveLayout();
  }

  QMainWindow::closeEvent(event);
}

void MainWindow::on_sandbox_tabs_tabCloseRequested(int index) {
  // removeTab() seems to be automatic when deleting page widgets
  delete ui->sandbox_tabs->widget(index);
  updateUi();
}

void MainWindow::on_action_new_sandbox_triggered() {
  auto snapshot = darwin::evolution()->snapshot();
  auto domain_ui_factory = snapshot.experiment->domainUiFactory();
  CHECK(domain_ui_factory);

  auto sandbox_window = domain_ui_factory->newSandboxWindow();
  if (sandbox_window != nullptr) {
    auto new_tab_index =
        ui->sandbox_tabs->addTab(sandbox_window, sandbox_window->windowTitle());
    ui->sandbox_tabs->setCurrentIndex(new_tab_index);
    updateUi();
  }
}

void MainWindow::on_action_reset_triggered() {
  CHECK(experiment_);

  // TODO: only prompt if active or modified
  if (!confirmEndOfExperiment())
    return;
    
  resetExperiment();
  updateUi();
}

void MainWindow::on_action_open_triggered() {
  if (experiment_ && !confirmEndOfExperiment())
    return;

  QString dir = QString::fromStdString(pal::userHomePath());
  if (!g_settings.last_directory.empty())
    dir = QString::fromStdString(g_settings.last_directory);

  QString path = QFileDialog::getOpenFileName(
      this, "Open existing universe", dir, "Darwin universe (*.darwin)");

  if (!path.isEmpty()) {
    closeExperiment();

    try {
      universe_ = darwin::Universe::open(path.toStdString());
      universeSwitched();
    } catch (const std::exception& e) {
      QMessageBox::warning(this, "Can't open universe", e.what());
    }
  }

  updateUi();
}

void MainWindow::on_action_new_triggered() {
  if (experiment_ && !confirmEndOfExperiment())
    return;

  QString dir_name = QString::fromStdString(pal::userHomePath());
  if (!g_settings.last_directory.empty())
    dir_name = QString::fromStdString(g_settings.last_directory);

  QString path_name = QFileDialog::getSaveFileName(
      this, "Create a new universe", dir_name, "Darwin universe (*.darwin)");

  if (!path_name.isEmpty()) {
    auto path = fs::path(path_name.toStdString());

    // automatically append the .darwin extension if needed
    // (this is needed since the Qt file open dialog on Linux doesn't do it)
    if (path.extension().empty()) {
      path.replace_extension(".darwin");
    }

    closeExperiment();

    try {
      universe_ = darwin::Universe::create(path.string());
      universeSwitched();
    } catch (const std::exception& e) {
      QMessageBox::warning(this, "Can't create universe", e.what());
    }
  }

  updateUi();
}

void MainWindow::on_action_new_experiment_triggered() {
  if (experiment_ && !confirmEndOfExperiment())
    return;

  NewExperimentDialog dlg(this, universe_.get(), nullptr);
  if (dlg.exec() != QDialog::Accepted)
    return;

  closeExperiment();

  try {
    darwin::ExperimentSetup setup;
    setup.population_size = dlg.populationSize();
    setup.population_name = dlg.populationName().toStdString();
    setup.domain_name = dlg.domainName().toStdString();
    setup.population_hint = dlg.populationHint();
    setup.domain_hint = dlg.domainHint();

    optional<string> name;
    if (!dlg.experimentName().isEmpty())
      name = dlg.experimentName().toStdString();

    experiment_ = make_shared<darwin::Experiment>(name, setup, nullopt, universe_.get());

    createExperimentWindows();
  } catch (const std::exception& e) {
    QMessageBox::warning(this, "Can't create experiment", e.what());
  }

  updateUi();
}

void MainWindow::on_action_open_experiment_triggered() {
  if (experiment_ && !confirmEndOfExperiment())
    return;

  OpenExperimentDialog dlg(this, universe_.get());
  if (dlg.exec() != QDialog::Accepted)
    return;

  closeExperiment();

  try {
    auto db_experiment = dlg.experiment();
    CHECK(db_experiment != nullptr);

    experiment_ = make_shared<darwin::Experiment>(db_experiment, universe_.get());

    // TODO: factor the common "open experiment" code
    createExperimentWindows();
  } catch (const std::exception& e) {
    QMessageBox::warning(this, "Can't create experiment", e.what());
  }

  updateUi();
}

void MainWindow::on_action_branch_experiment_triggered() {
  CHECK(experiment_);

  if (!confirmEndOfExperiment())
    return;

  NewExperimentDialog dlg(this, universe_.get(), experiment_.get());
  if (dlg.exec() != QDialog::Accepted)
    return;

  // HACK: make sure the parent experiment has a variation saved
  //
  // TODO: revisit and update the relevant interfaces
  //
  // TODO: if pending modifications, ask to save/discard first?
  //
  experiment_->save();

  auto base_variation_id = experiment_->dbVariationId();

  closeExperiment();

  try {
    darwin::ExperimentSetup setup;
    setup.population_size = dlg.populationSize();
    setup.population_name = dlg.populationName().toStdString();
    setup.domain_name = dlg.domainName().toStdString();
    setup.population_hint = dlg.populationHint();
    setup.domain_hint = dlg.domainHint();

    optional<string> name;
    if (!dlg.experimentName().isEmpty())
      name = dlg.experimentName().toStdString();

    experiment_ =
        make_shared<darwin::Experiment>(name, setup, base_variation_id, universe_.get());

    createExperimentWindows();
  } catch (const std::exception& e) {
    QMessageBox::warning(this, "Can't fork experiment", e.what());
  }

  updateUi();
}

void MainWindow::on_action_run_triggered() {
  auto evolution = darwin::evolution();

  // are we starting the experiment?
  if (!active_experiment_) {
    CHECK(experiment_);
    CHECK(batch_total_runs_ == 0);
    CHECK(batch_current_run_ == 0);

    StartEvolutionDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted)
      return;

    emit sigStartingExperiment();

    evolution_config_.copyFrom(dlg.evolutionConfig());
    active_experiment_ = evolution->newExperiment(experiment_, evolution_config_);
    batch_total_runs_ = dlg.batchRuns();
    batch_current_run_ = 1;
  }

  // start/resume
  if (active_experiment_)
    evolution->run();

  updateUi();
}

void MainWindow::on_action_pause_triggered() {
  darwin::evolution()->pause();
  updateUi();
}
