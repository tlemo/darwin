
#include "scene1.h"
#include "scene6.h"

#include "main_window.h"
#include "sandbox_window.h"
#include "ui_main_window.h"

MainWindow::MainWindow() : QMainWindow(nullptr), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  connect(ui->tabs, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
  initWorlds();
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::initWorlds() {
  for (const auto& [name, factory] : scenesRegistry()) {
    auto sandbox_window = make_unique<SandboxWindow>(factory.get());
    ui->tabs->addTab(sandbox_window.release(), QString::fromStdString(name));
  }
}

void MainWindow::onTabChanged(int index) {
  if (index != -1) {
    ui->tabs->currentWidget()->setFocus();
  }
}
