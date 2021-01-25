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
static auto randomElem(T& container) {
  random_device rd;
  default_random_engine rnd(rd());
  uniform_int_distribution<size_t> dist(0, container.size() - 1);
  return container.begin() + dist(rnd);
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

Genotype::Genotype(const Genotype& other) {
  root_ = deepCopy(other.root_);
  CHECK(segments_.size() == other.segments_.size());
}

Genotype& Genotype::operator=(const Genotype& other) {
  Genotype copy(other);
  std::swap(*this, copy);
  return *this;
}

unique_ptr<experimental::replicators::Phenotype> Genotype::grow() const {
  return make_unique<Phenotype>(this);
}

void Genotype::mutate() {
  // pick a random segment
  const auto segment = randomElem(segments_)->get();

  struct MutationType {
    double probability = 0;
    function<void()> mutate;
  };

  vector<MutationType> mutagen = {
    { 10, [&] { axialSplit(segment, 0.5); } },
    { 20, [&] { axialSplit(segment, 1.0); } },
    { 50, [&] { lateralSplit(segment, 0.5); } },
    { 50, [&] { lateralSplit(segment, 1.0); } },
    { 5, [&] { segment->suppressed = !segment->suppressed; } },
    { 50, [&] { growAppendage(segment); } },
    { 25, [&] { growSideAppendage(segment); } },
    { 100, [&] { mutateLength(segment, 0.5); } },
    { 100, [&] { mutateWidth(segment, 0.5); } },
    { 50, [&] { mutateSliceWidth(segment, 0.5); } },
  };

  double total = 0;
  for (const auto& mutation_type : mutagen) {
    CHECK(mutation_type.probability >= 0);
    total += mutation_type.probability;
  }
  CHECK(total > 0);

  random_device rd;
  default_random_engine rnd(rd());
  uniform_real_distribution<double> dist(0, total);

  for (int i = 0; i < kMutationCount; ++i) {
    const double sample = dist(rnd);
    double prefix_sum = 0;
    for (const auto& mutation_type : mutagen) {
      prefix_sum += mutation_type.probability;
      if (sample < prefix_sum) {
        mutation_type.mutate();
        break;
      }
    }
  }
}

Segment* Genotype::deepCopy(const Segment* segment) {
  if (segment == nullptr) {
    return nullptr;
  }

  unordered_map<const Segment*, Segment*> orig_to_clone;

  // do a DFS and a shallow copy of all reachable segments
  vector<const Segment*> stack = { segment };
  while (!stack.empty()) {
    const auto s = stack.back();
    stack.pop_back();
    if (orig_to_clone.find(s) == orig_to_clone.end()) {
      orig_to_clone.insert({ s, newSegment(*s) });

      if (s->side_appendage != nullptr) {
        stack.push_back(s->side_appendage);
      }

      for (const auto& slice : s->slices) {
        if (slice.appendage != nullptr) {
          stack.push_back(s->side_appendage);
        }
      }
    }
  }

  // fixup references
  for (auto kv : orig_to_clone) {
    const auto clone = kv.second;
    if (clone->side_appendage != nullptr) {
      clone->side_appendage = orig_to_clone.at(clone->side_appendage);
    }
    for (auto& slice : clone->slices) {
      if (slice.appendage != nullptr) {
        slice.appendage = orig_to_clone.at(slice.appendage);
      }
    }
  }

  return orig_to_clone.at(segment);
}

void Genotype::growAppendage(Segment* segment) {
  auto& slice = *randomElem(segment->slices);
  slice.appendage = newSegment(slice.appendage);
}

void Genotype::growSideAppendage(Segment* segment) {
  segment->side_appendage = newSegment(segment->side_appendage);
}

// pick a random slice and split it
void Genotype::lateralSplit(Segment* segment, double fraction) {
  CHECK(fraction > 0);

  const auto slice_it = randomElem(segment->slices);
  const auto new_width = slice_it->relative_width * fraction;

  slice_it->relative_width = new_width;

  Slice new_slice;
  new_slice.relative_width = new_width;
  new_slice.appendage = deepCopy(slice_it->appendage);

  segment->slices.insert(slice_it, new_slice);
}

// split the segment into two segments, chained together
void Genotype::axialSplit(Segment* segment, double fraction) {
  CHECK(fraction > 0);
  const auto new_length = segment->length * fraction;
  for (auto& slice : segment->slices) {
    slice.appendage = newSegment(slice.appendage);
    slice.appendage->length = new_length;
  }
  segment->length = new_length;
}

void Genotype::mutateLength(Segment* segment, double std_dev) {
  segment->length = mutateValue(segment->length, std_dev);
}

void Genotype::mutateWidth(Segment* segment, double std_dev) {
  segment->width = mutateValue(segment->width, std_dev);
}

void Genotype::mutateSliceWidth(Segment* segment, double std_dev) {
  auto& slice = *randomElem(segment->slices);
  slice.relative_width = mutateValue(slice.relative_width, std_dev);
}

unique_ptr<experimental::replicators::Genotype> Genotype::clone() const {
  return make_unique<Genotype>(*this);
}

}  // namespace experimental::replicators::seg_tree
