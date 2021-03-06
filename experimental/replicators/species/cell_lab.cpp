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
#include <core/math_2d.h>

#include <QPainter>
#include <QBrush>
#include <QColor>
#include <QPen>
#include <QPointF>

#include <functional>
using namespace std;

namespace experimental::replicators::cell_lab {

class Factory : public SpeciesFactory {
 public:
  unique_ptr<experimental::replicators::Genotype> primordialGenotype() override {
    return make_unique<Genotype>();
  }

  vector<unique_ptr<experimental::replicators::Genotype>> samples() override {
    vector<unique_ptr<experimental::replicators::Genotype>> samples;
    samples.push_back(primordialGenotype());
    samples.push_back(triangle());
    return samples;
  }

  void runTests() override {
    // TODO
    return;

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
      // TODO
      genotype.grow();
    }
  }

 private:
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

    auto root = genotype->root();
    root->daughter_1_gene = gene_a.index();
    root->daughter_2_gene = gene_b.index();

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

Phenotype::Phenotype(const Genotype* genotype) {
  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  b2Body* body = world_.CreateBody(&body_def);

  b2CircleShape shape;
  shape.m_radius = 0.1;

  b2FixtureDef fixture_def;
  fixture_def.shape = &shape;
  fixture_def.density = 1.0;
  body->CreateFixture(&fixture_def);
}

void Phenotype::animate() {
  experimental::replicators::Phenotype::animate();
}

void Phenotype::render(QPainter& painter, const QRectF& /*viewport*/) {
  painter.setPen(QPen(Qt::lightGray, 0, Qt::DotLine));
  painter.setBrush(Qt::NoBrush);
  painter.drawEllipse(QPointF(0, 0), 1, 1);
}

QRectF Phenotype::adjustViewport(const QRectF& viewport) {
  constexpr double kMargin = 1.0;
  return viewport.adjusted(-kMargin, kMargin, kMargin, -kMargin);
}

void swap(Genotype& a, Genotype& b) noexcept {
  using std::swap;
  swap(a.genes_, b.genes_);
  swap(a.root_, b.root_);
}

Genotype::Genotype() {
  genes_ = { Gene() };
}

unique_ptr<experimental::replicators::Phenotype> Genotype::grow() const {
  return make_unique<Phenotype>(this);
}

void Genotype::mutate() {
  // TODO
}

json Genotype::save() const {
  json json_obj;
  // TODO
  return json_obj;
}

void Genotype::load(const json& json_obj) {
  Genotype tmp;
  // TODO
  swap(*this, tmp);
}

bool Genotype::operator==(const Genotype& other) const {
  return genes_ == other.genes_ && root_ == other.root_;
}

Gene Genotype::randomGene() const {
  Gene gene;
  gene.split_angle = core::randomReal(-math::kPi, math::kPi);
  gene.daughter_1_angle = core::randomReal(-math::kPi, math::kPi);
  gene.daughter_2_angle = core::randomReal(-math::kPi, math::kPi);
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
  clone->root_ = root_;
  return clone;
}

}  // namespace experimental::replicators::cell_lab
