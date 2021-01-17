
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

QString NewExperimentDialog::speciesName() const {
  return ui->species_name->currentText();
}

}  // namespace experimental::replicators
