
#include "new_experiment_dialog.h"
#include "ui_new_experiment_dialog.h"
#include "replicators.h"

namespace experimental::replicators {

NewExperimentDialog::NewExperimentDialog(QWidget* parent, const char* title, bool samples)
    : QDialog(parent,
              Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
      ui(new Ui::NewExperimentDialog) {
  ui->setupUi(this);
  setWindowTitle(title);

  if (samples) {
    ui->population_size->setDisabled(true);
  }

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
    emit sigNewExperiment(ui->species_name->currentText(), ui->population_size->value());
  }
  QDialog::done(result);
}

}  // namespace experimental::replicators
