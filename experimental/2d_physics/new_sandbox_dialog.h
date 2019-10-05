
#pragma once

#include <QDialog>

namespace Ui {
class NewSandboxDialog;
}

class NewSandboxDialog : public QDialog {
  Q_OBJECT

 public:
  explicit NewSandboxDialog(QWidget* parent);
  ~NewSandboxDialog();
  
  QString sceneName() const;

 private:
  Ui::NewSandboxDialog* ui = nullptr;
};
