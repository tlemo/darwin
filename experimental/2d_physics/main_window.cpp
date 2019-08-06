
#include "scene1.h"
#include "scene6.h"

#include "main_window.h"
#include "sandbox_window.h"
#include "ui_main_window.h"

MainWindow::MainWindow() : QMainWindow(nullptr), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  connect(ui->tabs, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
  initWorld();
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::initWorld() {
  sandbox_factory_ = make_unique<sandbox_scene_1::Factory>();
  auto sandbox_window = make_unique<SandboxWindow>(sandbox_factory_.get());
  ui->tabs->addTab(sandbox_window.release(),
                   QString::fromStdString(sandbox_window->name()));
}

void MainWindow::onTabChanged(int index) {
  if (index != -1) {
    ui->tabs->currentWidget()->setFocus();
  }
}
