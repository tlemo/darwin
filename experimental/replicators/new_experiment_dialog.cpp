
#include "new_experiment_dialog.h"
#include "ui_new_experiment_dialog.h"
#include "replicators.h"

namespace experimental::replicators {

NewExperimentDialog::NewExperimentDialog(QWidget* parent, const char* title)
    : QDialog(parent,
              Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
      ui(new Ui::NewExperimentDialog) {
  ui->setupUi(this);
  setWindowTitle(title);

  for (const auto& [name, factory] : *registry()) {
    ui->species_name->addItem(QString::fromStdString(name));
  }

  ui->species_name->setFocus();
}

NewExperimentDialog::~NewExperimentDialog() {
  delete ui;
}

void NewExperimentDialog::done(int result) {
  if (result == QDialog::Accepted) {
    emit sigNewExperiment(ui->species_name->currentText());
  }
  QDialog::done(result);
}

}  // namespace experimental::replicators
