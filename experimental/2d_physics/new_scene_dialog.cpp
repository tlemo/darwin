
#include "new_scene_dialog.h"
#include "ui_new_scene_dialog.h"

NewSceneDialog::NewSceneDialog(QWidget* parent, core::PropertySet* config)
    : QDialog(parent,
              Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
      ui(new Ui::NewSceneDialog) {
  ui->setupUi(this);
  ui->properties->addPropertiesSection("Scene Configuration", config);
  ui->properties->autoSizeColumns();
}

NewSceneDialog::~NewSceneDialog() {
  delete ui;
}
