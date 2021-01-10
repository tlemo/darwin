
#include "main_window.h"
#include "ui_main_window.h"

#include <core_ui/sim/box2d_widget.h>

MainWindow::MainWindow() : QMainWindow(nullptr), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  layout_ = new QGridLayout(ui->central_widget);

  // testing...
  for (int row = 0; row < 2; ++row) {
    for (int col = 0; col < 2; ++col) {
      auto content = new physics_ui::Box2dWidget(ui->central_widget);
      content->setBorderSize(4);
      content->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
      layout_->addWidget(content, row, col);
    }
  }
}

MainWindow::~MainWindow() {
  delete ui;
}
