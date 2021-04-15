
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

class PhenotypeWidget;

class ExperimentWindow : public QFrame {
  Q_OBJECT

  static constexpr int kColumns = 4;
  static constexpr int kDefaultPopulationSize = 16;

  static constexpr int kDefaultTimerSpeed = 20;  // ms

 public:
  explicit ExperimentWindow(QWidget* parent, SpeciesFactory* factory, bool sample_set);
  ~ExperimentWindow();

  void refreshCandidates();

  bool animated() const { return animated_; }
  void setAnimated(bool animated);

  bool debugRender() const { return debug_render_; }
  void setDebugRender(bool debug_render);

  void setActive(bool active);

 private:
  void resetPopulation();
  void newGeneration(unique_ptr<Genotype> parent);
  void sampleGeneration();
  void pickGenotype(size_t index);
  void deletePhenotypeWidgets();
  void createPhenotypeWidgets();
  void onAnimationTimer();

 private:
  Ui::ExperimentWindow* ui = nullptr;
  QGridLayout* layout_ = nullptr;

  unique_ptr<Genotype> parent_;
  vector<unique_ptr<Genotype>> population_;

  // non-owning list of phenotype widgets
  vector<PhenotypeWidget*> phenotype_widgets_;

  SpeciesFactory* factory_ = nullptr;
  const bool sample_set_ = false;

  bool animated_ = true;
  bool debug_render_ = false;

  QTimer timer_;
};

}  // namespace experimental::replicators
