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

#include "cell_lab.h"

#include <core/global_initializer.h>
#include <core/exception.h>
#include <core/utils.h>
#include <core/random.h>
#include <core/chronometer.h>

#include <QPainter>
#include <QBrush>
#include <QColor>
#include <QPen>
#include <QPointF>
#include <QString>

#include <functional>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <tuple>
#include <math.h>
#include <limits>
using namespace std;

namespace experimental::replicators::cell_lab {

class Factory : public SpeciesFactory {
 public:
  unique_ptr<experimental::replicators::Genotype> primordialGenotype() override {
    return make_unique<Genotype>();
  }

  vector<unique_ptr<experimental::replicators::Genotype>> samples() override {
    vector<unique_ptr<experimental::replicators::Genotype>> samples;
#if 1
    samples.push_back(primordialGenotype());
    samples.push_back(single_cell());
    samples.push_back(triangle());
    samples.push_back(square());
    samples.push_back(spiral1());
    samples.push_back(spiral2());
    samples.push_back(exp1());
    samples.push_back(exp2());
    samples.push_back(exp3());
    samples.push_back(exp4());
    samples.push_back(exp5());
    samples.push_back(exp6());
    samples.push_back(mirrored());
#else
    samples.push_back(mirrored());
#endif
    return samples;
  }

  void runTests() override {
    // serialization roundtrip
    for (const auto& sample : samples()) {
      Genotype clone;
      clone.load(sample->save());
      CHECK(clone == dynamic_cast<Genotype&>(*sample));
      clone.grow();
    }

    // generating phenotypes
    for (const auto& sample : samples()) {
      sample->grow();
    }

    // cloning & deep copy
    for (const auto& sample : samples()) {
      auto clone = sample->clone();
      const auto clone_a = clone->clone();
      const auto clone_b = sample->clone();
      clone.reset();
      CHECK(dynamic_cast<Genotype&>(*clone_a) == dynamic_cast<Genotype&>(*clone_b));
      clone_a->grow();
      clone_b->grow();
    }

    // mutations
    for (const auto& sample : samples()) {
      auto& genotype = dynamic_cast<Genotype&>(*sample);
      genotype.swapDaughterGenes();
      genotype.swapDaughterAngles();
      genotype.swapDaughterMirror();
      genotype.swapDaughterCells();
      genotype.mutateTerminal();
      genotype.mutateSplitAngle(1.0);
      genotype.mutateDaughterAngle(1.0);
      genotype.mutateDaughterMirroring();
      genotype.mutateDaughterGene();
      genotype.duplicateDaughterGene();
      genotype.swapRandomGenes();
      genotype.mutateNewGene();
      genotype.mutateSeed();
      genotype.grow();
    }
  }

 private:
  unique_ptr<experimental::replicators::Genotype> single_cell() {
    auto genotype = make_unique<Genotype>();

    auto seed = genotype->seed();
    seed->terminal = true;

    return genotype;
  }

  unique_ptr<experimental::replicators::Genotype> triangle() {
    auto genotype = make_unique<Genotype>();

    auto gene_c = genotype->newGene(b2Color(1.0, 0.0, 0.0));
    gene_c->terminal = true;

    auto gene_b = genotype->newGene(b2Color(0.0, 1.0, 0.0));
    gene_b->daughter_1_gene = gene_c.index();
    gene_b->daughter_2_gene = gene_c.index();
    gene_b->split_angle = math::kPi / 2;

    auto gene_a = genotype->newGene(b2Color(0.0, 0.0, 1.0));
    gene_a->terminal = true;

    auto seed = genotype->seed();
    seed->daughter_1_gene = gene_a.index();
    seed->daughter_2_gene = gene_b.index();

    return genotype;
  }

  unique_ptr<experimental::replicators::Genotype> square() {
    auto genotype = make_unique<Genotype>();

    auto gene_c = genotype->newGene(b2Color(1.0, 0.0, 0.0));
    gene_c->terminal = true;

    auto gene_b = genotype->newGene(b2Color(0.0, 1.0, 0.0));
    gene_b->daughter_1_gene = gene_c.index();
    gene_b->daughter_2_gene = gene_c.index();
    gene_b->split_angle = math::kPi / 2;

    auto gene_a = genotype->newGene(b2Color(0.0, 0.0, 1.0));
    gene_a->daughter_1_gene = gene_c.index();
    gene_a->daughter_2_gene = gene_c.index();
    gene_a->split_angle = math::kPi / 2;

    auto seed = genotype->seed();
    seed->daughter_1_gene = gene_a.index();
    seed->daughter_2_gene = gene_b.index();

    return genotype;
  }

  unique_ptr<experimental::replicators::Genotype> spiral1() {
    auto genotype = make_unique<Genotype>();

    auto gene_b = genotype->newGene(b2Color(1.0, 0.0, 0.0));
    gene_b->terminal = true;

    auto seed = genotype->seed();
    seed->daughter_1_gene = seed.index();
    seed->daughter_1_angle = math::kPi / 4;
    seed->daughter_2_gene = gene_b.index();

    return genotype;
  }

  unique_ptr<experimental::replicators::Genotype> spiral2() {
    auto genotype = make_unique<Genotype>();

    auto gene_b = genotype->newGene(b2Color(1.0, 0.0, 0.0));
    gene_b->terminal = true;

    auto seed = genotype->seed();
    seed->daughter_1_gene = gene_b.index();
    seed->daughter_2_gene = seed.index();
    seed->split_angle = math::degreesToRadians(105);

    return genotype;
  }

  unique_ptr<experimental::replicators::Genotype> exp1() {
    auto genotype = make_unique<Genotype>();

    auto seed = genotype->seed();
    seed->split_angle = math::kPi / 4;

    return genotype;
  }

  unique_ptr<experimental::replicators::Genotype> exp2() {
    auto genotype = make_unique<Genotype>();

    auto seed = genotype->seed();
    seed->split_angle = math::kPi / 4;
    seed->daughter_1_angle = -math::kPi / 2;

    return genotype;
  }

  unique_ptr<experimental::replicators::Genotype> exp3() {
    auto genotype = make_unique<Genotype>();

    auto seed = genotype->seed();
    seed->split_angle = math::kPi / 2;

    return genotype;
  }

  unique_ptr<experimental::replicators::Genotype> exp4() {
    auto genotype = make_unique<Genotype>();

    auto seed = genotype->seed();

    auto gene_a = genotype->newGene(b2Color(0.0, 0.0, 1.0));
    gene_a->daughter_1_gene = gene_a.index();
    gene_a->daughter_2_gene = seed.index();
    gene_a->split_angle = math::kPi / 2;

    seed->daughter_1_gene = seed.index();
    seed->daughter_2_gene = gene_a.index();
    seed->split_angle = math::kPi / 2;

    return genotype;
  }

  unique_ptr<experimental::replicators::Genotype> exp5() {
    auto genotype = make_unique<Genotype>();

    auto gene_b = genotype->newGene(b2Color(1.0, 0.0, 0.0));
    gene_b->terminal = true;

    auto seed = genotype->seed();
    seed->daughter_1_gene = gene_b.index();
    seed->daughter_2_gene = seed.index();
    seed->split_angle = math::degreesToRadians(135);

    return genotype;
  }

  unique_ptr<experimental::replicators::Genotype> exp6() {
    auto genotype = make_unique<Genotype>();

    auto seed = genotype->seed();

    auto gene_a = genotype->newGene(b2Color(1.0, 0.0, 1.0));
    gene_a->daughter_1_gene = seed.index();
    gene_a->daughter_2_gene = gene_a.index();

    seed->daughter_1_gene = seed.index();
    seed->daughter_2_gene = gene_a.index();

    return genotype;
  }

  unique_ptr<experimental::replicators::Genotype> mirrored() {
    auto genotype = make_unique<Genotype>();

    auto gene_a = genotype->newGene(b2Color(0.0, 1.0, 0.5));
    gene_a->terminal = true;

    auto gene_b = genotype->newGene(b2Color(0.0, 0.5, 0.5));
    gene_b->daughter_1_gene = gene_b.index();
    gene_b->daughter_2_gene = gene_b.index();
    gene_b->split_angle = math::degreesToRadians(75);

    auto gene_c = genotype->newGene(b2Color(1.0, 0.5, 0.5));
    gene_c->daughter_1_gene = gene_c.index();
    gene_c->daughter_2_gene = gene_b.index();

    auto gene_d = genotype->newGene(b2Color(1.0, 1.0, 0.0));
    gene_d->daughter_1_gene = gene_c.index();
    gene_d->daughter_2_gene = gene_c.index();
    gene_d->daughter_1_angle = math::degreesToRadians(240);
    gene_d->daughter_1_mirror = true;
    gene_d->daughter_2_angle = math::degreesToRadians(300);
    gene_d->split_angle = math::kPi / 2;

    auto seed = genotype->seed();
    seed->daughter_1_gene = gene_a.index();
    seed->daughter_2_gene = gene_d.index();

    return genotype;
  }
};

GLOBAL_INITIALIZER {
  registry()->add<Factory>("CellLab");
}

bool Gene::operator==(const Gene& other) const {
  return split_angle == other.split_angle && daughter_1_angle == other.daughter_1_angle &&
         daughter_2_angle == other.daughter_2_angle &&
         daughter_1_gene == other.daughter_1_gene &&
         daughter_2_gene == other.daughter_2_gene && terminal == other.terminal;
}

Phenotype::Phenotype(const Genotype* genotype) : genotype_(*genotype) {
  grid_.resize(kGridExtent * kGridExtent);
  world_.SetAllowSleeping(false);
  cells_.push_back(createCell(genotype_.seed().ptr(), math::Vector2d(0, 0), 0));
}

// TODO: move to math2d + unit tests
Scalar relativeAngle(Scalar ref, Scalar angle) {
  const auto raw_diff = fmodf(angle - ref, math::kPi * 2);
  return raw_diff > math::kPi
             ? raw_diff - math::kPi * 2
             : raw_diff < -math::kPi ? raw_diff + math::kPi * 2 : raw_diff;
}

bool Phenotype::simulationStep(Scalar dt) {
  CHECK(dt > 0);

  core::Chronometer scope_timing(&stats_.elapsed_substeps, true);

  Scalar max_angular_velocity = 0;
  Scalar max_velocity_squared = 0;

  for (auto& cell : cells_) {
    const Scalar velocity_squared = cell.velocity.lengthSquared();
    max_velocity_squared = max(max_velocity_squared, velocity_squared);
    max_angular_velocity = max(max_angular_velocity, cell.angular_velocity);
    cell.pos = cell.pos + cell.velocity * dt;
    cell.velocity = cell.velocity * kLinearDamping;
    cell.angle += cell.angular_velocity * dt;
    cell.angular_velocity *= kAngularDamping;
  }

  return max_angular_velocity > 0.01 || max_velocity_squared > 0.001;
}

void Phenotype::constraints() {
  core::Chronometer scope_timing(&stats_.elapsed_constraints, true);

  for (int i = 0; i < int(cells_.size()); ++i) {
    auto& cell = cells_[i];
    for (const auto& adhesion : cell.adhesions) {
      auto& other_cell = cells_[adhesion.other_cell_index];

      // orientation constraint
      constexpr Scalar kTorqueFactor = 1.0;
      const auto other_pos = other_cell.pos - cell.pos;
      const auto actual_angle = atan2(other_pos.y, other_pos.x) - cell.angle;
      const auto rel_angle = relativeAngle(adhesion.angle, actual_angle);
      cell.angular_velocity += rel_angle * kTorqueFactor;

      // position constraint (try to move the connected cell)
      constexpr Scalar kSepDistFactor = 0.8;
      constexpr Scalar kRelPosFactor = 0.6;
      const auto target_dist = (cell.size + other_cell.size) * kSepDistFactor;
      const auto a = adhesion.angle + cell.angle;
      const auto target_pos = math::Vector2d(cos(a), sin(a)) * target_dist + cell.pos;
      const auto rel_pos = target_pos - other_cell.pos;
      const auto imp = rel_pos * kRelPosFactor;
      cell.velocity = cell.velocity - imp;
      other_cell.velocity = other_cell.velocity + imp;
    }
  }
}

void Phenotype::handleCollisionPair(Cell& cell_a, Cell& cell_b) {
  ++stats_.collision_tests;
  const auto d = cell_b.pos - cell_a.pos;
  const auto dist = d.length();
  constexpr Scalar kSepDistFactor = 0.9;
  const auto sep_dist = dist - (cell_a.size + cell_b.size) * kSepDistFactor;
  if (sep_dist < 0) {
    ++stats_.actual_collisions;
    constexpr Scalar kSepFactor = 1.0;
    const auto magnitude = -sep_dist * kSepFactor;
    if (magnitude > 0.01) {
      const auto imp = d * (magnitude / dist);
      cell_a.velocity = cell_a.velocity - imp;
      cell_b.velocity = cell_b.velocity + imp;
    }
  }
}

void Phenotype::collisions() {
  core::Chronometer scope_timing(&stats_.elapsed_collisions, true);

#if 0
  for (int i = 0; i < int(cells_.size()); ++i) {
    for (int j = i + 1; j < int(cells_.size()); ++j) {
      handleCollisionPair(cells_[i], cells_[j]);
    }
  }
#else
  // reset spatial hashing
  for (auto& bin : grid_) {
    bin.clear();
  }

  const auto gridCoord = [](Scalar pos) {
    constexpr Scalar kGridOrigin = -(kGridExtent * kGridCellSize) / 2;
    const auto coord = int(floor((pos - kGridOrigin) / kGridCellSize));
    return max(min(coord, kGridExtent - 1), 0);
  };

  // place cells into the grid bins
  for (int cell_index = 0; cell_index < int(cells_.size()); ++cell_index) {
    const auto& cell = cells_[cell_index];
    const auto& pos = cell.pos;
    const auto aabb_size = cell.size;
    const int first_row = gridCoord(pos.y - aabb_size);
    const int last_row = gridCoord(pos.y + aabb_size);
    const int first_col = gridCoord(pos.x - aabb_size);
    const int last_col = gridCoord(pos.x + aabb_size);
    for (int i = first_row; i <= last_row; ++i) {
      for (int j = first_col; j <= last_col; ++j) {
        const auto grid_index = i * kGridExtent + j;
        grid_[grid_index].push_back(cell_index);
      }
    }
  }

  const auto cells_count = cells_.size();
  vector<bool> pairs(cells_count * cells_count);

  // check for collisions between the cells in each bin
  for (const auto& close_cells : grid_) {
    for (size_t i = 1; i < close_cells.size(); ++i) {
      for (size_t j = 0; j < i; ++j) {
        const auto index_a = close_cells[i];
        const auto index_b = close_cells[j];
        const auto pair_index = index_a * cells_count + index_b;
        if (!pairs[pair_index]) {
          pairs[pair_index] = true;
          handleCollisionPair(cells_[index_a], cells_[index_b]);
        }
      }
    }
  }
#endif
}

void Phenotype::cellLifecycle() {
  core::Chronometer scope_timing(&stats_.elapsed_lifecycle, true);

  constexpr Scalar kTimeStep = 0.1;
  timestamp_ += kTimeStep;

  // we may append new cells, so remember the original count
  const int cell_count = int(cells_.size());

  for (int i = 0; i < cell_count; ++i) {
    auto& cell = cells_[i];
    const auto& gene = *cell.gene;
    const auto age = timestamp_ - cell.mitosis_timestamp;
    CHECK(age > 0);

    // cell growth
    CHECK(cell.size >= kNucleusRadius);
    cell.size = min(cell.size + kGrowthRate, kMaxCellSize);

    const Scalar angle_mirror = cell.mirrored ? -1 : 1;

    // cell division?
    // TODO: customize division age
    if (age > kDivisionAge && !gene.terminal && cells_.size() < kMaxCellCount) {
      last_division_timestamp_ = timestamp_;

      const auto split_angle = gene.split_angle * angle_mirror + cell.angle;
      const auto rel_pos =
          math::Vector2d(cos(split_angle), sin(split_angle)) * (cell.size / 4);

      // daughter 1 will reuse old cell
      const auto daughter_1_gene = genotype_[gene.daughter_1_gene].ptr();
      const auto daughter_1_angle = split_angle + gene.daughter_1_angle * angle_mirror;
      const auto daughter_1_pos = cell.pos - rel_pos;
      auto daughter_1 = createCell(daughter_1_gene, daughter_1_pos, daughter_1_angle);
      daughter_1.size = max(cell.size / 2, kNucleusRadius);
      daughter_1.mirrored = cell.mirrored != gene.daughter_1_mirror;
      const int daughter_1_index = i;

      // daughter 2 will be appended as a new cell
      const auto daughter_2_gene = genotype_[gene.daughter_2_gene].ptr();
      const auto daughter_2_angle = split_angle + gene.daughter_2_angle * angle_mirror;
      const auto daughter_2_pos = cell.pos + rel_pos;
      auto daughter_2 = createCell(daughter_2_gene, daughter_2_pos, daughter_2_angle);
      daughter_2.size = max(cell.size / 2, kNucleusRadius);
      daughter_2.mirrored = cell.mirrored != gene.daughter_2_mirror;
      const int daughter_2_index = int(cells_.size());

      // new adhesion between daughter cells
      const auto d1_rel = daughter_2_pos - daughter_1_pos;
      const auto d2_rel = daughter_1_pos - daughter_2_pos;
      daughter_1.adhesions.push_back(
          { daughter_2_index, Scalar(atan2(d1_rel.y, d1_rel.x) - daughter_1_angle) });
      daughter_2.adhesions.push_back(
          { daughter_1_index, Scalar(atan2(d2_rel.y, d2_rel.x) - daughter_2_angle) });

      // inherit & cleanup old adhesions
      for (const auto& old_adhesion : cell.adhesions) {
        auto& other_cell = cells_[old_adhesion.other_cell_index];

        // find and erase counterpart adhesion
        const auto other_adhesion_it = std::find_if(
            other_cell.adhesions.begin(),
            other_cell.adhesions.end(),
            [i](const auto& adhesion) { return adhesion.other_cell_index == i; });
        CHECK(other_adhesion_it != other_cell.adhesions.end());
        const auto other_angle = other_adhesion_it->angle;
        other_cell.adhesions.erase(other_adhesion_it);

        constexpr auto margin = math::kPi / 32;
        const auto other_pos = other_cell.pos - cell.pos;
        const auto actual_angle = atan2(other_pos.y, other_pos.x);
        const auto split_line_angle = split_angle + math::kPi / 2;
        const auto rel_angle = relativeAngle(split_line_angle, actual_angle);

        // inherit into daughter_1?
        if (rel_angle > -margin || rel_angle < -math::kPi + margin) {
          Adhesion inherited_adhesion;
          const auto angle_transf = cell.angle - daughter_1.angle;
          inherited_adhesion.angle = old_adhesion.angle + angle_transf;
          inherited_adhesion.other_cell_index = old_adhesion.other_cell_index;
          daughter_1.adhesions.push_back(inherited_adhesion);

          Adhesion counterpart_adhesion;
          counterpart_adhesion.angle = other_angle;
          counterpart_adhesion.other_cell_index = daughter_1_index;
          other_cell.adhesions.push_back(counterpart_adhesion);
        }

        // inherit into daughter 2?
        if (rel_angle < margin || rel_angle > math::kPi - margin) {
          Adhesion inherited_adhesion;
          const auto angle_transf = cell.angle - daughter_2.angle;
          inherited_adhesion.angle = old_adhesion.angle + angle_transf;
          inherited_adhesion.other_cell_index = old_adhesion.other_cell_index;
          daughter_2.adhesions.push_back(inherited_adhesion);

          Adhesion counterpart_adhesion;
          counterpart_adhesion.angle = other_angle;
          counterpart_adhesion.other_cell_index = daughter_2_index;
          other_cell.adhesions.push_back(counterpart_adhesion);
        }
      }

      swap(cell, daughter_1);
      cells_.push_back(daughter_2);
    }
  }
}

void Phenotype::animate() {
  stats_ = {};

  core::Chronometer scope_timing(&stats_.elapsed_animate);

  if (timestamp_ - last_division_timestamp_ < kExtraTime) {
    for (int i = 0; i < kSimulationIterations; ++i) {
      cellLifecycle();
      constraints();
      collisions();
      if (!simulationStep(kSimulationDt) && cells_.size() == kMaxCellCount) {
        break;
      }
    }
  }
}

static QPointF vecToPoint(const math::Vector2d& v) {
  return QPointF(v.x, v.y);
}

void Phenotype::renderStats(QPainter& painter) {
  QString text;

  text += QString::asprintf("Timestamp: %.2f\n", timestamp_);

  if (verbose_stats_) {
    const auto& box2d_profile = world_.GetProfile();
    text += QString::asprintf("Box2d.step: %.3f ms\n", box2d_profile.step);
    text += QString::asprintf("Box2d.objects_count: %d\n", world_.GetBodyCount());
    text += QString::asprintf("Box2d.collide: %.3f ms\n", box2d_profile.collide);
    text += QString::asprintf("Box2d.solve: %.3f ms\n", box2d_profile.solve);
    text += QString::asprintf("Box2d.broadphase: %.3f ms\n", box2d_profile.broadphase);
    text += QString::asprintf("Box2d.solveTOI: %.3f ms\n", box2d_profile.solveTOI);
  }

  text += QString::asprintf("Cells count: %zu\n", cells_.size());
  text += QString::asprintf("FPS: %.2f\n", fps_tracker_.currentRate());
  text += QString::asprintf("Total: %.3f ms\n", stats_.elapsed_animate);
  text += QString::asprintf("Substeps: %.3f ms\n", stats_.elapsed_substeps);
  text += QString::asprintf("Cell lifecycle: %.3f ms\n", stats_.elapsed_lifecycle);
  text += QString::asprintf("Constraints: %.3f ms\n", stats_.elapsed_constraints);
  text += QString::asprintf("Collisions: %.3f ms\n", stats_.elapsed_collisions);

  if (verbose_stats_) {
    text += QString::asprintf("Collision tests: %d\n", stats_.collision_tests);
    text += QString::asprintf("Actual collisions: %d\n", stats_.actual_collisions);
  }

  text += QString::asprintf("Constraint iter: %d", kSimulationIterations);

  painter.save();
  painter.resetTransform();
  painter.setFont(QFont("Arial", 10, QFont::Normal));

  auto font_metrics = painter.fontMetrics();
  auto bounding_rect = font_metrics.boundingRect(QRect(10, 10, 1000, 1000), 0, text);

  painter.setPen(QPen(QColor(128, 128, 128, 128), 0));
  painter.setBrush(QColor(196, 240, 196, 128));
  painter.drawRect(bounding_rect.adjusted(-4, -4, 4, 4));

  painter.setPen(QColor(0, 0, 0, 200));
  painter.drawText(bounding_rect, 0, text);
  painter.restore();
}

void Phenotype::render(QPainter& painter, const QRectF& /*viewport*/, bool debug) {
  fps_tracker_.update();

  // perf hack: trade off rendering quality for speed when we have a large number of cells
  if (cells_.size() > 32) {
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, false);
  }

  for (int i = 0; i < int(cells_.size()); ++i) {
    const auto& cell = cells_[i];

    // cell
    const auto& color = cell.gene->color;
    painter.setPen(QPen(Qt::darkGray, 0.1));
    painter.setBrush(QColor::fromRgbF(color.r, color.g, color.b, 0.3));
    painter.drawEllipse(vecToPoint(cell.pos), cell.size, cell.size);

    // nucleus
    const auto nucleus_size = kNucleusRadius * cell.size;
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor::fromRgbF(0.2, 0.2, 0.2, 0.5));
    painter.drawEllipse(vecToPoint(cell.pos), nucleus_size, nucleus_size);

    if (debug) {
      // split angle
      painter.setPen(QPen(Qt::black, 0, Qt::DotLine));
      const auto split_angle = cell.angle + cell.gene->split_angle + math::kPi / 2;
      const auto split_v = math::Vector2d(cos(split_angle), sin(split_angle)) * cell.size;
      painter.drawLine(vecToPoint(cell.pos - split_v), vecToPoint(cell.pos + split_v));

      // x-axis
      const auto x_axis_v = math::Vector2d(cos(cell.angle), sin(cell.angle)) * cell.size;
      painter.setPen(QPen(Qt::darkGray, 0.05));
      painter.drawLine(vecToPoint(cell.pos), vecToPoint(cell.pos + x_axis_v));

      // adhesions (reference angle)
      painter.setPen(QPen(Qt::green, 0.06, Qt::DotLine));
      for (auto adhesion : cell.adhesions) {
        const auto angle = adhesion.angle + cell.angle;
        const auto v = math::Vector2d(cos(angle), sin(angle)) * cell.size;
        painter.drawLine(vecToPoint(cell.pos), vecToPoint(cell.pos + v));
      }
    }

    // adhesions (linked cells)
    if (debug) {
      painter.setPen(QPen(Qt::red, 0.02));
      for (auto adhesion : cell.adhesions) {
        if (adhesion.other_cell_index > i) {
          const auto& other_cell = cells_[adhesion.other_cell_index];
          painter.drawLine(vecToPoint(cell.pos), vecToPoint(other_cell.pos));
        }
      }
    }
  }

  if (debug) {
    renderStats(painter);
  }
}

QRectF Phenotype::adjustViewport(const QRectF& /*viewport*/) {
  Scalar min_x = -2.0;
  Scalar max_x = 2.0;
  Scalar min_y = -2.0;
  Scalar max_y = 2.0;

  for (const auto& cell : cells_) {
    min_x = min(min_x, Scalar(cell.pos.x - cell.size));
    max_x = max(max_x, Scalar(cell.pos.x + cell.size));
    min_y = min(min_y, Scalar(cell.pos.y - cell.size));
    max_y = max(max_y, Scalar(cell.pos.y + cell.size));
  }

  CHECK(min_x < max_x);
  CHECK(min_y < max_y);

  return QRectF(min_x, max_y, max_x - min_x, min_y - max_y);
}

Cell Phenotype::createCell(const Gene* gene, const math::Vector2d& pos, Scalar angle) {
  Cell cell;
  cell.gene = gene;
  cell.size = 0.5;  // TODO
  cell.mitosis_timestamp = timestamp_;
  cell.pos = pos;
  cell.angle = angle;
  return cell;
}

void swap(Genotype& a, Genotype& b) noexcept {
  using std::swap;
  swap(a.genes_, b.genes_);
  swap(a.seed_, b.seed_);
}

Genotype::Genotype() {
  genes_ = { Gene() };
}

Genotype::Genotype(const Genotype& other) : genes_(other.genes_), seed_(other.seed_) {}

Genotype& Genotype::operator=(const Genotype& other) {
  Genotype copy(other);
  swap(*this, copy);
  return *this;
}

unique_ptr<experimental::replicators::Phenotype> Genotype::grow() const {
  return make_unique<Phenotype>(this);
}

void Genotype::mutate() {
  struct MutationType {
    double weight = 0;
    function<void()> mutate;
  };

  vector<MutationType> mutagen = {
    MutationType{ 10, [&] { swapDaughterGenes(); } },
    MutationType{ 10, [&] { swapDaughterAngles(); } },
    MutationType{ 10, [&] { swapDaughterMirror(); } },
    MutationType{ 10, [&] { swapDaughterCells(); } },
    MutationType{ 10, [&] { mutateTerminal(); } },
    MutationType{ 10, [&] { mutateSplitAngle(0.5); } },
    MutationType{ 10, [&] { mutateDaughterAngle(0.5); } },
    MutationType{ 10, [&] { mutateDaughterMirroring(); } },
    MutationType{ 10, [&] { mutateDaughterGene(); } },
    MutationType{ 10, [&] { duplicateDaughterGene(); } },
    MutationType{ 10, [&] { swapRandomGenes(); } },
    MutationType{ 10, [&] { mutateNewGene(); } },
    MutationType{ 10, [&] { mutateSeed(); } },
  };

  core::randomWeightedElem(mutagen)->mutate();
}

json Genotype::save() const {
  json json_obj;
  json_obj["seed"] = seed_;
  json_obj["genes"] = genes_;
  return json_obj;
}

void Genotype::load(const json& json_obj) {
  Genotype tmp;
  tmp.seed_ = json_obj.at("seed");
  tmp.genes_ = json_obj.at("genes").get<vector<Gene>>();
  swap(*this, tmp);
}

void Genotype::swapDaughterGenes() {
  auto& gene = *core::randomElem(genes_);
  std::swap(gene.daughter_1_gene, gene.daughter_2_gene);
}

void Genotype::swapDaughterAngles() {
  auto& gene = *core::randomElem(genes_);
  std::swap(gene.daughter_1_angle, gene.daughter_2_angle);
}

void Genotype::swapDaughterMirror() {
  auto& gene = *core::randomElem(genes_);
  std::swap(gene.daughter_1_mirror, gene.daughter_2_mirror);
}

void Genotype::swapDaughterCells() {
  auto& gene = *core::randomElem(genes_);
  std::swap(gene.daughter_1_gene, gene.daughter_2_gene);
  std::swap(gene.daughter_1_angle, gene.daughter_2_angle);
  std::swap(gene.daughter_1_mirror, gene.daughter_2_mirror);
}

void Genotype::mutateTerminal() {
  auto& gene = *core::randomElem(genes_);
  gene.terminal = !gene.terminal;
}

void Genotype::mutateSplitAngle(Scalar std_dev) {
  auto& gene = *core::randomElem(genes_);
  gene.split_angle = core::mutateNormalValue(gene.split_angle, std_dev);
}

void Genotype::mutateDaughterAngle(Scalar std_dev) {
  auto& gene = *core::randomElem(genes_);
  auto& angle = core::randomCoin() ? gene.daughter_1_angle : gene.daughter_2_angle;
  angle = core::mutateNormalValue(angle, std_dev);
}

void Genotype::mutateDaughterMirroring() {
  auto& gene = *core::randomElem(genes_);
  auto& mirror = core::randomCoin() ? gene.daughter_1_mirror : gene.daughter_2_mirror;
  mirror = !mirror;
}

void Genotype::mutateDaughterGene() {
  auto& gene = *core::randomElem(genes_);
  auto& daughter_gene = core::randomCoin() ? gene.daughter_1_gene : gene.daughter_2_gene;
  daughter_gene = core::randomInteger(0, int(genes_.size()));
}

void Genotype::duplicateDaughterGene() {
  const int gene_index = core::randomInteger(0, int(genes_.size()));

  auto gene_clone = newGene();

  auto& gene = genes_[gene_index];
  auto& daughter_gene = core::randomCoin() ? gene.daughter_1_gene : gene.daughter_2_gene;

  *gene_clone = genes_[daughter_gene];
  gene_clone->color.r = core::randomReal(0.0, 1.0);
  gene_clone->color.g = core::randomReal(0.0, 1.0);
  gene_clone->color.b = core::randomReal(0.0, 1.0);

  daughter_gene = gene_clone.index();
}

void Genotype::swapRandomGenes() {
  auto& gene_a = *core::randomElem(genes_);
  auto& gene_b = *core::randomElem(genes_);
  std::swap(gene_a, gene_b);
}

void Genotype::mutateNewGene() {
  const int gene_index = core::randomInteger(0, int(genes_.size()));

  auto new_gene = newGene(randomGene());

  auto& gene = genes_[gene_index];
  auto& daughter_gene = core::randomCoin() ? gene.daughter_1_gene : gene.daughter_2_gene;
  daughter_gene = new_gene.index();
}

void Genotype::mutateSeed() {
  seed_ = core::randomInteger(0, int(genes_.size()));
}

bool Genotype::operator==(const Genotype& other) const {
  return genes_ == other.genes_ && seed_ == other.seed_;
}

Gene Genotype::randomGene() const {
  Gene gene;
  gene.split_angle = core::randomReal(-math::kPi, math::kPi);
  gene.daughter_1_angle = core::randomReal(-math::kPi, math::kPi);
  gene.daughter_2_angle = core::randomReal(-math::kPi, math::kPi);
  gene.daughter_1_mirror = core::randomCoin();
  gene.daughter_2_mirror = core::randomCoin();
  gene.daughter_1_gene = core::randomInteger(0, int(genes_.size()));
  gene.daughter_2_gene = core::randomInteger(0, int(genes_.size()));
  gene.terminal = core::randomCoin(0.2);
  gene.color.r = core::randomReal(0.0, 1.0);
  gene.color.g = core::randomReal(0.0, 1.0);
  gene.color.b = core::randomReal(0.0, 1.0);
  return gene;
}

unique_ptr<experimental::replicators::Genotype> Genotype::clone() const {
  auto clone = make_unique<Genotype>();
  clone->genes_ = genes_;
  clone->seed_ = seed_;
  return clone;
}

}  // namespace experimental::replicators::cell_lab
