
#pragma once

#include <core_ui/properties_widget.h>

#include <QMainWindow>
#include <QFrame>
#include <QDockWidget>

namespace Ui {
class MainWindow;
}

namespace experimental::replicators {

class ExperimentWindow;

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow();
  ~MainWindow() override;

 private slots:
  void on_action_new_experiment_triggered();
  void on_action_close_tab_triggered();
  void on_action_new_sample_set_triggered();
  void on_tabs_tabCloseRequested(int index);
  void on_tabs_currentChanged(int index);
  void on_action_refresh_candidates_triggered();
  void on_action_animate_phenotypes_toggled(bool checked);
  void on_action_debug_render_toggled(bool checked);

 private:
  void newExperimentWindow(bool sample_set);
  void openTab(const QString& species_name, bool sample_set);
  void closeTab(int index);
  void enableExperimentActions(bool enabled);
  void deactivateCurrentExperiment();

  void dockWindow(const char* name,
                  QFrame* window,
                  Qt::DockWidgetAreas allowed_areas,
                  Qt::DockWidgetArea area);

 private:
  Ui::MainWindow* ui = nullptr;
  ExperimentWindow* current_experiment_window_ = nullptr;
};

}  // namespace experimental::replicators
