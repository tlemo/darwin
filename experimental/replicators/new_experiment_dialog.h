
#pragma once

#include <QDialog>

namespace Ui {
class NewExperimentDialog;
}

namespace experimental::replicators {

class NewExperimentDialog : public QDialog {
  Q_OBJECT

 public:
  explicit NewExperimentDialog(QWidget* parent, const char* title);
  ~NewExperimentDialog();

  void done(int result) override;

 signals:
  void sigNewExperiment(QString species_name);

 private:
  Ui::NewExperimentDialog* ui = nullptr;
};

}  // namespace experimental::replicators
