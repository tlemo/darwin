
#pragma once

#include <QMainWindow>

#include <memory>
#include <vector>
using namespace std;

namespace Ui {
class MainWindow;
}

class ToolWindow;
class SandboxWindow;

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow();
  ~MainWindow() override;
  
  void updateToolWindows();

 private:
  void dockWindow(ToolWindow* tool_window,
                  Qt::DockWidgetAreas allowed_areas,
                  Qt::DockWidgetArea area);

 private:
  Ui::MainWindow* ui = nullptr;
  vector<ToolWindow*> tool_windows_;
};

