
#include "new_experiment_dialog.h"
#include "ui_new_experiment_dialog.h"
#include "replicators.h"

NewExperimentDialog::NewExperimentDialog(QWidget* parent)
    : QDialog(parent,
              Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
      ui(new Ui::NewExperimentDialog) {
  ui->setupUi(this);

  for (const auto& [name, factory] : *experimental::replicators::registry()) {
    ui->species_name->addItem(QString::fromStdString(name));
  }

  ui->species_name->setFocus();
}

NewExperimentDialog::~NewExperimentDialog() {
  delete ui;
}

QString NewExperimentDialog::speciesName() const {
  return ui->species_name->currentText();
}
