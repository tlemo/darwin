
#pragma once

#include "world.h"

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QSlider>

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
  void zoomIn();
  void zoomOut();

  void updateZoom();

  void setupToolbar();

  void dockWindow(ToolWindow* tool_window,
                  Qt::DockWidgetAreas allowed_areas,
                  Qt::DockWidgetArea area);

  void simStep();

 private:
  Ui::MainWindow* ui = nullptr;
  vector<ToolWindow*> tool_windows_;
  World world_;
  QTimer timer_;
  QSlider* zoom_slider = nullptr;
  QLabel* status_label = nullptr;
};
