
#pragma once

#include "replicators.h"

#include <QFrame>
#include <QGridLayout>
#include <QTimer>

#include <vector>
#include <memory>
using namespace std;

namespace Ui {
class ExperimentWindow;
}

namespace experimental::replicators {

class ExperimentWindow : public QFrame {
  Q_OBJECT

  static constexpr int kColumns = 4;
  static constexpr int kDefaultPopulationSize = 16;

  static constexpr int kDefaultTimerSpeed = 20;  // ms

 public:
  explicit ExperimentWindow(QWidget* parent, SpeciesFactory* factory, bool sample_set);
  ~ExperimentWindow();

  void refreshCandidates();

 private:
  void resetPopulation();
  void newGeneration(unique_ptr<Genotype> parent);
  void sampleGeneration();
  void pickGenotype(size_t index);
  void deletePhenotypeWidgets();
  void createPhenotypeWidgets();

 private:
  Ui::ExperimentWindow* ui = nullptr;
  QGridLayout* layout_ = nullptr;

  unique_ptr<Genotype> parent_;
  vector<unique_ptr<Genotype>> population_;

  SpeciesFactory* factory_ = nullptr;
  const bool sample_set_ = false;

  QTimer timer_;
};

}  // namespace experimental::replicators
