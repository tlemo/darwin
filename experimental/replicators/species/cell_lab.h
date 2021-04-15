// Copyright The Darwin Neuroevolution Framework Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "replicators.h"

#include <core/rate_tracker.h>
#include <core/math_2d.h>
#include <third_party/box2d/box2d.h>

#include <vector>
#include <utility>
#include <memory>
using namespace std;

// CellLab (https://cell-lab.net) inspired morphology

namespace experimental::replicators::cell_lab {

using Scalar = float;

struct Gene {
  Scalar split_angle = 0;
  Scalar daughter_1_angle = 0;
  Scalar daughter_2_angle = 0;
  bool daughter_1_mirror = false;
  bool daughter_2_mirror = false;
  int daughter_1_gene = 0;
  int daughter_2_gene = 0;
  bool terminal = false;
  b2Color color{ 0.5, 1.0, 0.5 };

  Gene() = default;

  Gene(const b2Color& color) : color(color) {}

  bool operator==(const Gene& other) const;

  JSON_DEF(Gene,
           split_angle,
           daughter_1_angle,
           daughter_2_angle,
           daughter_1_mirror,
           daughter_2_mirror,
           daughter_1_gene,
           daughter_2_gene,
           terminal,
           color.r,
           color.g,
           color.b)
};

class Genotype : public experimental::replicators::Genotype {
 public:
  Genotype();

  Genotype(const Genotype& other);
  Genotype& operator=(const Genotype& other);

  friend void swap(Genotype& a, Genotype& b) noexcept;

  unique_ptr<experimental::replicators::Phenotype> grow() const override;
  unique_ptr<experimental::replicators::Genotype> clone() const override;
  void mutate() override;

  json save() const override;
  void load(const json& json_obj) override;

  auto seed() { return ArrayElem(genes_, seed_); }

  auto operator[](int index) { return ArrayElem(genes_, index); }

  template <class... Args>
  auto newGene(Args&&... args) {
    genes_.emplace_back(std::forward<Args>(args)...);
    return ArrayElem(genes_, genes_.size() - 1);
  }

  // mutations
  void swapDaughterGenes();
  void swapDaughterAngles();
  void swapDaughterMirror();
  void swapDaughterCells();
  void mutateTerminal();
  void mutateSplitAngle(Scalar std_dev);
  void mutateDaughterAngle(Scalar std_dev);
  void mutateDaughterMirroring();
  void mutateDaughterGene();
  void duplicateDaughterGene();
  void swapRandomGenes();
  void mutateNewGene();
  void mutateSeed();

  // for testing purposes
  bool operator==(const Genotype& other) const;

 private:
  Gene randomGene() const;

 private:
  vector<Gene> genes_;
  int seed_ = 0;
};

struct Cell;

struct Adhesion {
  int other_cell_index = -1;
  Scalar angle = 0;
};

struct Cell {
  const Gene* gene = nullptr;
  math::Vector2d pos;
  math::Vector2d velocity;
  Scalar angle = 0;
  Scalar angular_velocity = 0;
  Scalar size = 0;
  Scalar mitosis_timestamp = 0;
  bool mirrored = false;
  vector<Adhesion> adhesions;
};

class Phenotype : public experimental::replicators::Phenotype {
  static constexpr int kMaxCellCount = 256;
  static constexpr Scalar kNucleusRadius = 0.1;
  static constexpr Scalar kMaxCellSize = 2.0;

  static constexpr Scalar kGrowthRate = 0.05;
  static constexpr Scalar kDivisionAge = 30.0;

  static constexpr int kSimulationIterations = 100;
  static constexpr Scalar kSimulationDt = 1.0 / 10.0;
  static constexpr Scalar kExtraTime = kDivisionAge * 5;

  static constexpr Scalar kLinearDamping = 0.1;
  static constexpr Scalar kAngularDamping = 0.1;

  static constexpr Scalar kGridCellSize = 2.0;
  static constexpr int kGridExtent = 100;

  // CONSIDER: max, avg?
  struct Stats {
    double elapsed_constraints = 0;
    double elapsed_collisions = 0;
    double elapsed_lifecycle = 0;
    double elapsed_substeps = 0;
    double elapsed_animate = 0;
    int collision_tests = 0;
    int actual_collisions = 0;
  };

 public:
  explicit Phenotype(const Genotype* genotype);

  // replicators::Phenotype interface
  void animate() override;

  // physics_ui::Box2dSceneUi interface
  void render(QPainter& painter, const QRectF& viewport, bool debug) override;
  QRectF adjustViewport(const QRectF& viewport) override;

 private:
  Cell createCell(const Gene* gene, const math::Vector2d& pos, Scalar angle);
  bool simulationStep(Scalar dt);
  void constraints();
  void collisions();
  void cellLifecycle();
  void handleCollisionPair(Cell& cell_a, Cell& cell_b);
  void renderStats(QPainter& painter);

 private:
  Genotype genotype_;
  vector<Cell> cells_;

  core::RateTracker fps_tracker_;
  Stats stats_;

  // spatial hashing grid
  vector<vector<int>> grid_;

  Scalar last_division_timestamp_ = 0;

  bool verbose_stats_ = false;
};

}  // namespace experimental::replicators::cell_lab
