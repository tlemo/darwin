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

#include "segment_tree.h"

#include <core/global_initializer.h>

namespace experimental::replicators::seg_tree {

class Factory : public SpeciesFactory {
 public:
  unique_ptr<experimental::replicators::Genotype> primordialGenotype() override {
    return make_unique<Genotype>();
  }

  vector<unique_ptr<experimental::replicators::Genotype>> samples() override {
    return {};
  }
};

GLOBAL_INITIALIZER {
  registry()->add<Factory>("Segment Tree");
}

Phenotype::Phenotype() {
  // TODO
}

b2World* Phenotype::specimen() const {
  // TODO
  return nullptr;
}

void Phenotype::animate() {
  // TODO
}

Genotype::Genotype() {
  // TODO
}

unique_ptr<experimental::replicators::Phenotype> Genotype::grow() const {
  // TODO
  return nullptr;
}

unique_ptr<experimental::replicators::Genotype> Genotype::mutate() const {
  // TODO
  return nullptr;
}

}  // namespace experimental::replicators::seg_tree
