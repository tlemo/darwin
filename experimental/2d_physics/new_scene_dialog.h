
#pragma once

#include <core/properties.h>

#include <QDialog>

namespace Ui {
class NewSceneDialog;
}

class NewSceneDialog : public QDialog {
  Q_OBJECT

 public:
  explicit NewSceneDialog(QWidget* parent, core::PropertySet* config);
  ~NewSceneDialog();

 private:
  Ui::NewSceneDialog* ui = nullptr;
};
