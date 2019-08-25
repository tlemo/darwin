
#include "main_window.h"
#include "new_sandbox_dialog.h"
#include "sandbox_window.h"
#include "ui_main_window.h"

MainWindow::MainWindow() : QMainWindow(nullptr), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  connect(ui->tabs, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::onTabChanged(int index) {
  if (index != -1) {
    ui->tabs->currentWidget()->setFocus();
  }
}

void MainWindow::on_action_new_sandbox_triggered() {
  NewSandboxDialog dlg(this);
  if (dlg.exec() == QDialog::Accepted) {
    const auto& scene_name = dlg.sceneName();
    const auto factory = scenesRegistry().find(scene_name.toStdString());
    CHECK(factory != nullptr);
    auto sandbox_window = make_unique<SandboxWindow>(factory);
    auto new_tab_index = ui->tabs->addTab(sandbox_window.release(), scene_name);
    ui->tabs->setCurrentIndex(new_tab_index);
  }
}
