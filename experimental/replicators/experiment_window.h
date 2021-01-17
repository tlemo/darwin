
#pragma once

#include "replicators.h"

#include <QFrame>
#include <QGridLayout>

namespace Ui {
class ExperimentWindow;
}

namespace experimental::replicators {

class ExperimentWindow : public QFrame {
  Q_OBJECT

 public:
  explicit ExperimentWindow(QWidget* parent, SpeciesFactory* factory);
  ~ExperimentWindow();

 private:
  Ui::ExperimentWindow* ui = nullptr;
  QGridLayout* layout_ = nullptr;

  SpeciesFactory* factory_ = nullptr;
};

}  // namespace experimental::replicators
