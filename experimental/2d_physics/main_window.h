
#pragma once

#include "sandbox_factory.h"

#include <QMainWindow>

#include <memory>
using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow();
  ~MainWindow();

 private:
  void initWorld();
  void onTabChanged(int index);

 private:
  Ui::MainWindow* ui = nullptr;
  unique_ptr<SandboxFactory> sandbox_factory_;
};
