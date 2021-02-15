
#pragma once

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

namespace experimental::replicators {

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
  void on_action_refresh_candidates_triggered();
  void on_action_animate_phenotypes_toggled(bool checked);
  void on_action_debug_render_toggled(bool checked);

 private:
  void newExperimentWindow(bool sample_set);
  void closeTab(int index);

 private:
  Ui::MainWindow* ui = nullptr;
};

}  // namespace experimental::replicators
