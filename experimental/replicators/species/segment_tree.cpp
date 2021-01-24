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
#include <random>
#include <functional>
using namespace std;

namespace experimental::replicators::seg_tree {

template <class T>
static auto& randomElem(T& container) {
  random_device rd;
  default_random_engine rnd(rd());
  uniform_int_distribution<size_t> dist(0, container.size() - 1);
  return container[dist(rnd)];
}

static double mutateValue(double value, double std_dev) {
  random_device rd;
  default_random_engine rnd(rd());
  normal_distribution<double> dist(value, std_dev);
  return dist(rnd);
}

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

void Genotype::mutate() {
  // pick a random segment
  auto& segment = randomElem(segments_);

  struct SegmentMutation {
    double probability = 0;
    function<void(Segment*)> mutation;
  };

  vector<SegmentMutation> mutagen = {
    { 10, [&](Segment* segment) { axialSplit(segment, 0.5); } },
    { 20, [&](Segment* segment) { axialSplit(segment, 1.0); } },
    { 50, [&](Segment* segment) { lateralSplit(segment, 0.5); } },
    { 50, [&](Segment* segment) { lateralSplit(segment, 1.0); } },
    { 5, [&](Segment* segment) { segment->suppressed = !segment->suppressed; } },
    { 50, [&](Segment* segment) { growAppendage(segment); } },
    { 25, [&](Segment* segment) { growSideAppendage(segment); } },
    { 100, [&](Segment* segment) { mutateLength(segment, 0.5); } },
    { 100, [&](Segment* segment) { mutateWidth(segment, 0.5); } },
    { 50, [&](Segment* segment) { mutateSliceWidth(segment, 0.5); } },
  };
}

void Genotype::growAppendage(Segment* segment) {
  auto& slice = randomElem(segment->slices);
  slice.appendage = newSegment(slice.appendage);
}

void Genotype::growSideAppendage(Segment* segment) {
  segment->side_appendage = newSegment(segment->side_appendage);
}

void Genotype::lateralSplit(Segment* segment, double fraction) {
  // TODO
}

// split the segment into two segments, chained together
void Genotype::axialSplit(Segment* segment, double fraction) {
  CHECK(fraction > 0 && fraction < 1);
  segment->length *= fraction;
  for (auto& slice : segment->slices) {
    slice.appendage = newSegment(slice.appendage);
    slice.appendage->length = segment->length * (1 - fraction);
  }
}

void Genotype::mutateLength(Segment* segment, double std_dev) {
  segment->length = mutateValue(segment->length, std_dev);
}

void Genotype::mutateWidth(Segment* segment, double std_dev) {
  segment->width = mutateValue(segment->width, std_dev);
}

void Genotype::mutateSliceWidth(Segment* segment, double std_dev) {
  auto& slice = randomElem(segment->slices);
  slice.relative_width = mutateValue(slice.relative_width, std_dev);
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

}  // namespace experimental::replicators::seg_tree
