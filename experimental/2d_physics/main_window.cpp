
#include "main_window.h"
#include "camera_window.h"
#include "touch_window.h"
#include "new_sandbox_dialog.h"
#include "sandbox_window.h"
#include "ui_main_window.h"

#include <QDockWidget>

MainWindow::MainWindow() : QMainWindow(nullptr), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  // main toolbar menu entry
  ui->menu_windows->addAction(ui->tool_bar->toggleViewAction());

  // configure docking
  setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
  setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
  setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
  setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

  // camera window
  auto camera_window = new CameraWindow(this);
  dockWindow(camera_window, Qt::AllDockWidgetAreas, Qt::BottomDockWidgetArea);
  
  // touch window
  auto touch_window = new TouchWindow(this);
  dockWindow(touch_window, Qt::AllDockWidgetAreas, Qt::LeftDockWidgetArea);
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::updateToolWindows() {
  for (auto tool_window : tool_windows_) {
    tool_window->update();
  }
}

void MainWindow::onSandboxChange(SandboxWindow* sandbox_window) {
  if (sandbox_window != nullptr) {
    sandbox_window->setFocus();
  }

  for (auto tool_window : tool_windows_) {
    tool_window->onSandboxChange(sandbox_window);
  }
}

void MainWindow::dockWindow(ToolWindow* tool_window,
                            Qt::DockWidgetAreas allowed_areas,
                            Qt::DockWidgetArea area) {
  auto dock = new QDockWidget;
  dock->setAllowedAreas(allowed_areas);
  dock->setWindowTitle(tool_window->windowTitle());
  dock->setMinimumSize(100, 75);
  dock->setWidget(tool_window);
  addDockWidget(area, dock);

  ui->menu_windows->addAction(dock->toggleViewAction());

  tool_windows_.push_back(tool_window);
}

void MainWindow::on_action_new_sandbox_triggered() {
  NewSandboxDialog dlg(this);
  if (dlg.exec() == QDialog::Accepted) {
    const auto& scene_name = dlg.sceneName();
    const auto factory = scenesRegistry().find(scene_name.toStdString());
    CHECK(factory != nullptr);
    auto sandbox_window = make_unique<SandboxWindow>(this, factory);
    auto new_tab_index = ui->tabs->addTab(sandbox_window.release(), scene_name);
    ui->tabs->setCurrentIndex(new_tab_index);
  }
}

void MainWindow::on_tabs_tabCloseRequested(int index) {
  delete ui->tabs->widget(index);
}

void MainWindow::on_tabs_currentChanged(int index) {
  SandboxWindow* new_sandbox_window = nullptr;
  if (index != -1) {
    new_sandbox_window = dynamic_cast<SandboxWindow*>(ui->tabs->currentWidget());
    Q_ASSERT(new_sandbox_window != nullptr);
  }
  onSandboxChange(new_sandbox_window);
}
