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

#include <unordered_map>
using namespace std;

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

Phenotype::Phenotype(const Genotype* genotype) {
  // TODO
  b2PolygonShape shape;
  shape.SetAsBox(2, 8);

  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  b2Body* body = world_.CreateBody(&body_def);

  b2FixtureDef fixture_def;
  fixture_def.shape = &shape;
  fixture_def.density = 1.0f;
  body->CreateFixture(&fixture_def);
}

Genotype::Genotype() {
  root_ = newSegment();
}

unique_ptr<experimental::replicators::Phenotype> Genotype::grow() const {
  return make_unique<Phenotype>(this);
}

unique_ptr<experimental::replicators::Genotype> Genotype::clone() const {
  auto clone = make_unique<Genotype>();

  unordered_map<Segment*, Segment*> orig_to_clone;

  // copy all segments
  for (const auto& segment : segments_) {
    auto segment_clone = make_unique<Segment>(*segment);
    CHECK(orig_to_clone.insert({ segment.get(), segment_clone.get() }).second);
    clone->segments_.push_back(std::move(segment_clone));
  }

  // fixup segment pointers
  for (auto& segment : clone->segments_) {
    if (segment->side_appendage) {
      segment->side_appendage = orig_to_clone.at(segment->side_appendage);
    }
    for (auto& slice : segment->slices) {
      if (slice.appendage != nullptr) {
        slice.appendage = orig_to_clone.at(slice.appendage);
      }
    }
  }

  // finally, set the expression root
  clone->root_ = orig_to_clone.at(root_);

  return clone;
}

void Genotype::mutate() {
  // TODO
}

}  // namespace experimental::replicators::seg_tree
