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

#include <vector>
#include <utility>
#include <memory>
using namespace std;

// CellLab (https://cell-lab.net) inspired morphology

namespace experimental::replicators::cell_lab {

struct Gene {
  double split_angle = 0;
  double daughter_1_angle = 0;
  double daughter_2_angle = 0;
  int daughter_1_gene = 0;
  int daughter_2_gene = 0;
  bool terminal = false;
  b2Color color{ 0.5, 1.0, 0.5 };

  Gene() = default;

  Gene(const b2Color& color) : color(color) {}

  bool operator==(const Gene& other) const;
};

class Genotype : public experimental::replicators::Genotype {
 public:
  Genotype();

  friend void swap(Genotype& a, Genotype& b) noexcept;

  unique_ptr<experimental::replicators::Phenotype> grow() const override;
  unique_ptr<experimental::replicators::Genotype> clone() const override;
  void mutate() override;

  json save() const override;
  void load(const json& json_obj) override;

  auto root() { return ArrayElem(genes_, root_); }

  template <class... Args>
  auto newGene(Args&&... args) {
    genes_.emplace_back(std::forward<Args>(args)...);
    return ArrayElem(genes_, genes_.size() - 1);
  }

  // mutations
  // TODO

  // for testing purposes
  bool operator==(const Genotype& other) const;

 private:
  Gene randomGene() const;

 private:
  vector<Gene> genes_;
  int root_ = 0;
};

class Phenotype : public experimental::replicators::Phenotype {
 public:
  explicit Phenotype(const Genotype* genotype);

  // replicators::Phenotype interface
  void animate() override;

  // physics_ui::Box2dSceneUi interface
  void render(QPainter& painter, const QRectF& viewport) override;
  QRectF adjustViewport(const QRectF& viewport) override;
};

}  // namespace experimental::replicators::cell_lab
