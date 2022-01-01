
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

  static constexpr int kDefaultZoom = 5;

 public:
  MainWindow();
  ~MainWindow() override;

  void updateToolWindows();

 private slots:
  void on_action_select_toggled(bool checked);
  void on_action_pan_view_toggled(bool checked);

 private:
  void zoomIn();
  void zoomOut();

  void updateZoom();

  void setupToolbar();
  void setupToolWindows();

  void dockWindow(ToolWindow* tool_window,
                  Qt::DockWidgetAreas allowed_areas,
                  Qt::DockWidgetArea area);

  void simStep();

 private:
  Ui::MainWindow* ui = nullptr;
  vector<ToolWindow*> tool_windows_;
  Simulation simulation_;
  QTimer timer_;
  QSlider* zoom_slider = nullptr;
  QLabel* status_label = nullptr;
};
