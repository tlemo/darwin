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
#include <array>
using namespace std;

namespace experimental::replicators::seg_tree {

template <class T>
static auto randomElem(T& container) {
  random_device rd;
  default_random_engine rnd(rd());
  uniform_int_distribution<size_t> dist(0, container.size() - 1);
  return container.begin() + dist(rnd);
}

static double mutateValue(double value, double std_dev, double min_value = 0.01) {
  random_device rd;
  default_random_engine rnd(rd());
  normal_distribution<double> dist(value, std_dev);
  return max(dist(rnd), min_value);
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
  createSegment(genotype->root(), nullptr, b2Vec2(), b2Vec2());
}

void Phenotype::createSegment(const Segment* segment,
                              b2Body* parent_body,
                              const b2Vec2& base_left,
                              const b2Vec2& base_right) {
  CHECK(segment != nullptr);
  CHECK(!segment->slices.empty());

  const auto d = base_right - base_left;

  // body
  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  body_def.position = (base_left + base_right) * 0.5f;
  body_def.angle = parent_body ? atan2f(d.y, d.x) : 0;
  b2Body* body = world_.CreateBody(&body_def);

  const float base_width = d.Length();

  float total_rel_width = 0;
  for (const auto& slice : segment->slices) {
    total_rel_width += slice.relative_width;
  }
  CHECK(total_rel_width > 0);

  // calculate base/extremity points
  float base_x = -(base_width / 2);
  float extremity_x = -(segment->width / 2);
  vector<b2Vec2> base_points = { b2Vec2(base_x, 0) };
  vector<b2Vec2> extremity_points = { b2Vec2(extremity_x, segment->length) };
  for (const auto& slice : segment->slices) {
    const auto fraction = slice.relative_width / total_rel_width;
    base_x += fraction * base_width;
    extremity_x += fraction * segment->width;
    base_points.emplace_back(base_x, 0);
    extremity_points.emplace_back(extremity_x, segment->length);
  }

  // adjust extremity points (fixed length from the base counterpart)
  for (size_t i = 0; i < extremity_points.size(); ++i) {
    const auto& bp = base_points[i];
    auto& ep = extremity_points[i];
    ep = (ep - bp).Normalized() * segment->length + bp;
  }

  // create fixtures for each of the slices
  CHECK(segment->slices.size() + 1 == base_points.size());
  CHECK(segment->slices.size() + 1 == extremity_points.size());
  for (size_t i = 0; i < segment->slices.size(); ++i) {
    array<b2Vec2, 4> points;
    points[0] = base_points[i];
    points[1] = extremity_points[i];
    points[2] = extremity_points[i + 1];
    points[3] = base_points[i + 1];

    b2PolygonShape shape;
    shape.Set(points.data(), points.size());

    b2FixtureDef fixture_def;
    fixture_def.shape = &shape;
    fixture_def.density = 1.0f;
    body->CreateFixture(&fixture_def);
  }

  // create appendages
  for (size_t i = 0; i < segment->slices.size(); ++i) {
    const auto& slice = segment->slices[i];
    if (slice.appendage) {
      createSegment(slice.appendage, body, extremity_points[i], extremity_points[i + 1]);
    }
  }

  // side appendage, if any
  if (segment->side_appendage) {
    // left
    createSegment(
        segment->side_appendage, body, base_points.front(), extremity_points.front());

    // right (mirror)
    createSegment(
        segment->side_appendage, body, extremity_points.back(), base_points.front());
  }

  // TODO: set joints
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
