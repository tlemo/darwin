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
    vector<unique_ptr<experimental::replicators::Genotype>> samples;
    samples.push_back(primordialGenotype());
    samples.push_back(slices());
    samples.push_back(appendages());
    samples.push_back(sideAppendage());
    samples.push_back(mirrorSideAppendage());
    return samples;
  }

 private:
  unique_ptr<experimental::replicators::Genotype> slices() {
    auto genotype = make_unique<Genotype>();
    genotype->root()->slices.push_back({ 2.0 });
    genotype->root()->slices.push_back({ 3.0 });
    genotype->root()->slices.push_back({ 0.5 });
    return genotype;
  }

  unique_ptr<experimental::replicators::Genotype> appendages() {
    auto genotype = make_unique<Genotype>();
    auto appendage = genotype->newSegment();
    appendage->length = 2.0;
    auto appendage2 = genotype->newSegment(appendage);
    appendage2->width = 0.5;
    genotype->root()->slices.push_back({ 2.0, appendage });
    genotype->root()->slices.push_back({ 1.0, appendage2 });
    return genotype;
  }

  unique_ptr<experimental::replicators::Genotype> sideAppendage() {
    auto genotype = make_unique<Genotype>();
    auto appendage = genotype->newSegment();
    genotype->root()->side_appendage = appendage;
    return genotype;
  }

  unique_ptr<experimental::replicators::Genotype> mirrorSideAppendage() {
    auto genotype = make_unique<Genotype>();

    auto level4 = genotype->newSegment();
    level4->length = 2.0;
    level4->width = 1.0;

    auto sss = genotype->newSegment();
    sss->slices.push_back({ 3.0, nullptr });
    sss->length = 3.0;
    sss->width = 0.5;

    auto ss_level2 = genotype->newSegment();
    ss_level2->length = 1.0;
    ss_level2->width = 0.5;
    ss_level2->side_appendage = sss;

    auto ss = genotype->newSegment();
    ss->slices.push_back({ 3.0, ss_level2 });
    ss->length = 3.0;
    ss->width = 0.5;

    auto side_level3 = genotype->newSegment();
    side_level3->length = 1.0;
    side_level3->width = 0.2;
    side_level3->side_appendage = ss;

    auto side_level2 = genotype->newSegment();
    side_level2->slices.push_back({ 3.0, side_level3 });
    side_level2->length = 2.0;
    side_level2->width = 2.0;
    side_level2->side_appendage = ss;

    auto side_level1 = genotype->newSegment();
    side_level1->slices.push_back({ 3.0, side_level2 });
    side_level1->length = 2.0;
    side_level1->width = 0.5;

    auto level3 = genotype->newSegment();
    level3->slices.push_back({ 3.0, level4 });
    level3->length = 0.5;
    level3->width = 0.5;
    level3->side_appendage = side_level1;

    auto level2 = genotype->newSegment();
    level2->slices.push_back({ 3.0, level3 });
    level2->length = 1.0;
    level2->width = 1.0;

    auto level1 = genotype->newSegment();
    level1->slices.push_back({ 3.0, level2 });
    level1->length = 0.2;
    level1->width = 1.0;

    genotype->root()->side_appendage = level1;

    return genotype;
  }
};

GLOBAL_INITIALIZER {
  registry()->add<Factory>("Segment Tree");
}

Phenotype::Phenotype(const Genotype* genotype) {
  createSegment(genotype->root(), nullptr, b2Vec2(), b2Vec2(), false);
}

void Phenotype::createSegment(const Segment* segment,
                              b2Body* parent_body,
                              const b2Vec2& base_left,
                              const b2Vec2& base_right,
                              bool mirror) {
  CHECK(segment != nullptr);
  CHECK(!segment->slices.empty());

  const auto d = base_right - base_left;

  // body
  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  body_def.position = (base_left + base_right) * 0.5f;
  body_def.angle = parent_body ? atan2f(d.y, d.x) : 0;
  b2Body* body = world_.CreateBody(&body_def);

  const float scale = parent_body ? d.Length() : 1.0f;
  const float base_width = parent_body ? 1.0f * scale : 0.0f;
  const float width = segment->width * scale;
  const float length = segment->length * scale;

  float total_rel_width = 0;
  for (const auto& slice : segment->slices) {
    total_rel_width += slice.relative_width;
  }
  CHECK(total_rel_width > 0);

  // calculate base/extremity points
  float base_x = -(base_width / 2);
  float extremity_x = -(width / 2);
  vector<b2Vec2> base_points = { b2Vec2(base_x, 0) };
  vector<b2Vec2> extremity_points = { b2Vec2(extremity_x, length) };
  for (const auto& slice : segment->slices) {
    const auto fraction = slice.relative_width / total_rel_width;
    base_x += fraction * base_width;
    extremity_x += fraction * width;
    base_points.emplace_back(base_x, 0);
    extremity_points.emplace_back(extremity_x, length);
  }

  // adjust extremity points (fixed length from the base counterpart)
  for (size_t i = 0; i < extremity_points.size(); ++i) {
    const auto& bp = base_points[i];
    auto& ep = extremity_points[i];
    ep = (ep - bp).Normalized() * length + bp;
  }

  // mirroring x coordinates?
  if (mirror) {
    for (auto& point : base_points) {
      point.x = -point.x;
    }
    for (auto& point : extremity_points) {
      point.x = -point.x;
    }
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
      auto left = body->GetWorldPoint(extremity_points[i]);
      auto right = body->GetWorldPoint(extremity_points[i + 1]);
      if (mirror) {
        swap(left, right);
      }
      createSegment(slice.appendage, body, left, right, mirror);
    }
  }

  // side appendage, if any
  if (segment->side_appendage) {
    // left appendage
    auto la_left = body->GetWorldPoint(base_points.front());
    auto la_right = body->GetWorldPoint(extremity_points.front());
    if (mirror) {
      swap(la_left, la_right);
    }
    createSegment(segment->side_appendage, body, la_left, la_right, mirror);

    // right appendage (mirror)
    auto ra_left = body->GetWorldPoint(extremity_points.back());
    auto ra_right = body->GetWorldPoint(base_points.back());
    if (mirror) {
      swap(ra_left, ra_right);
    }
    createSegment(segment->side_appendage, body, ra_left, ra_right, !mirror);
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
