
#pragma once

#include <QMainWindow>
#include <QGridLayout>

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

 private:
  Ui::MainWindow* ui = nullptr;
  QGridLayout* layout_ = nullptr;
};

}  // namespace experimental::replicators
