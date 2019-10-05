
#include "new_sandbox_dialog.h"
#include "sandbox_factory.h"
#include "ui_new_sandbox_dialog.h"

NewSandboxDialog::NewSandboxDialog(QWidget* parent)
    : QDialog(parent,
              Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
      ui(new Ui::NewSandboxDialog) {
  ui->setupUi(this);

  for (const auto& [name, factory] : scenesRegistry()) {
    ui->scene_name->addItem(QString::fromStdString(name));
  }
  
  ui->scene_name->setFocus();
}

NewSandboxDialog::~NewSandboxDialog() {
  delete ui;
}

QString NewSandboxDialog::sceneName() const {
  return ui->scene_name->currentText();
}
