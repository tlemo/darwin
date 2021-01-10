
#pragma once

#include <QMainWindow>
#include <QGridLayout>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow();
  ~MainWindow() override;

 private:
  Ui::MainWindow* ui = nullptr;
  QGridLayout* layout_ = nullptr;
};
