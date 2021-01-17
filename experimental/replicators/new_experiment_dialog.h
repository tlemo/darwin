
#pragma once

#include <QDialog>

namespace Ui {
class NewExperimentDialog;
}

class NewExperimentDialog : public QDialog {
  Q_OBJECT

 public:
  explicit NewExperimentDialog(QWidget* parent);
  ~NewExperimentDialog();

  QString speciesName() const;

 private:
  Ui::NewExperimentDialog* ui = nullptr;
};
