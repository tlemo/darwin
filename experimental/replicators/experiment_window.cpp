
#include "experiment_window.h"
#include "ui_experiment_window.h"
#include "phenotype_widget.h"

#include <core/exception.h>
#include <core/parallel_for_each.h>

namespace experimental::replicators {

ExperimentWindow::ExperimentWindow(QWidget* parent,
                                   SpeciesFactory* factory,
                                   int population_size,
                                   bool sample_set)
    : QFrame(parent),
      ui(new Ui::ExperimentWindow),
      factory_(factory),
      population_size_(population_size),
      sample_set_(sample_set) {
  ui->setupUi(this);
  setFocusPolicy(Qt::StrongFocus);
  layout_ = new QGridLayout(this);
  timer_.setInterval(kDefaultTimerSpeed);
  connect(&timer_, &QTimer::timeout, this, &ExperimentWindow::onAnimationTimer);
  resetPopulation();
}

ExperimentWindow::~ExperimentWindow() {
  timer_.stop();
  delete ui;
}

void ExperimentWindow::refreshCandidates() {
  if (parent_) {
    newGeneration(std::move(parent_));
  } else if (sample_set_) {
    sampleGeneration();
  }
}

void ExperimentWindow::setAnimated(bool animated) {
  animated_ = animated;
  if (animated_) {
    timer_.start();
  } else {
    timer_.stop();
  }
}

void ExperimentWindow::setDebugRender(bool debug_render) {
  debug_render_ = debug_render;
  for (int i = 0; i < layout_->count(); ++i) {
    const auto widget = dynamic_cast<PhenotypeWidget*>(layout_->itemAt(i)->widget());
    CHECK(widget != nullptr);
    widget->setDebugRender(debug_render_);
  }
}

void ExperimentWindow::setActive(bool active) {
  if (active && animated_) {
    timer_.start();
  } else {
    timer_.stop();
  }
}

void ExperimentWindow::resetPopulation() {
  if (sample_set_) {
    sampleGeneration();
  } else {
    newGeneration(factory_->primordialGenotype());
  }
}

void ExperimentWindow::newGeneration(unique_ptr<Genotype> parent) {
  deletePhenotypeWidgets();

  parent_ = std::move(parent);
  population_.clear();
  for (int i = 0; i < population_size_; ++i) {
    auto clone = parent_->clone();
    clone->mutate();
    population_.push_back(std::move(clone));
  }

  createPhenotypeWidgets();
}

void ExperimentWindow::sampleGeneration() {
  deletePhenotypeWidgets();

  parent_.reset();
  population_ = factory_->samples();
  if (population_.empty()) {
    throw core::Exception("Empty sample set");
  }

  createPhenotypeWidgets();
}

void ExperimentWindow::pickGenotype(size_t index) {
  newGeneration(population_[index]->clone());
}

void ExperimentWindow::onAnimationTimer() {
  CHECK(timer_.isActive());
  CHECK(animated_);

#ifdef DARWIN_OS_WASM
  // Emscripten support for threads is not functional yet
  for (auto widget : phenotype_widgets_) {
    widget->animate();
    widget->update();
  }
#else
  pp::for_each(phenotype_widgets_, [](int, auto widget) {
    // animate each widget in parallel
    widget->animate();
  });

  // we need to call update() from the UI thread
  for (auto widget : phenotype_widgets_) {
    widget->update();
  }
#endif
}

void ExperimentWindow::deletePhenotypeWidgets() {
  timer_.stop();
  phenotype_widgets_.clear();
  while (auto item = layout_->takeAt(0)) {
    delete item->widget();
    delete item;
  }
}

void ExperimentWindow::createPhenotypeWidgets() {
  CHECK(phenotype_widgets_.empty());

  // TODO: multi-threaded phenotype animation
  for (size_t i = 0; i < population_.size(); ++i) {
    const auto col = i % kColumns;
    const auto row = i / kColumns;
    const auto widget = new PhenotypeWidget(this, population_[i]->grow());
    widget->setDebugRender(debug_render_);
    connect(widget, &PhenotypeWidget::sigClicked, [=] { pickGenotype(i); });
    layout_->addWidget(widget, row, col);
    phenotype_widgets_.push_back(widget);
  }

  CHECK(!timer_.isActive());
  if (animated_) {
    timer_.start();
  }
}

}  // namespace experimental::replicators
