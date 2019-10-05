
#pragma once

#include "sandbox_factory.h"

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
  void onSandboxChange(SandboxWindow* sandbox_window);

 private slots:
  void on_action_new_sandbox_triggered();
  void on_tabs_tabCloseRequested(int index);
  void on_tabs_currentChanged(int index);
  
 private:
  void dockWindow(ToolWindow* tool_window,
                  Qt::DockWidgetAreas allowed_areas,
                  Qt::DockWidgetArea area);

 private:
  Ui::MainWindow* ui = nullptr;
  unique_ptr<SandboxFactory> sandbox_factory_;
  vector<ToolWindow*> tool_windows_;
};
