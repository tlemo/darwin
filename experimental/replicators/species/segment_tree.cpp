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
#include <core/exception.h>
#include <core/utils.h>

#include <unordered_map>
#include <random>
#include <functional>
#include <array>
#include <string>
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
    samples.push_back(suppressed());
    samples.push_back(appendages());
    samples.push_back(sideAppendage());
    samples.push_back(mirrorSideAppendage());
    samples.push_back(jsonDefinition());
    samples.push_back(fingers());
    samples.push_back(multipleBodySegments());
    samples.push_back(axialSplit());
    return samples;
  }

  void runTests() override {
    // serialization roundtrip
    for (const auto& sample : samples()) {
      Genotype clone;
      clone.load(sample->save());
      CHECK(clone == dynamic_cast<Genotype&>(*sample));
    }

    // cloning & deep copy
    for (const auto& sample : samples()) {
      auto clone = sample->clone();
      const auto clone_a = clone->clone();
      const auto clone_b = sample->clone();
      clone.reset();
      CHECK(dynamic_cast<Genotype&>(*clone_a) == dynamic_cast<Genotype&>(*clone_b));
    }

    // mutations
    for (const auto& sample : samples()) {
      auto& genotype = dynamic_cast<Genotype&>(*sample);
      genotype.growAppendage(genotype.root());
      genotype.growSideAppendage(genotype.root());
      genotype.lateralSplit(genotype.root(), 0.2);
      genotype.axialSplit(genotype.root(), 0.8);
      genotype.mutateLength(genotype.root(), 1.0);
      genotype.mutateWidth(genotype.root(), 1.0);
      genotype.mutateSliceWidth(genotype.root(), 1.0);
    }
  }

 private:
  unique_ptr<experimental::replicators::Genotype> slices() {
    auto genotype = make_unique<Genotype>();
    genotype->root()->slices.push_back({ 2.0 });
    genotype->root()->slices.push_back({ 4.0 });
    genotype->root()->slices.push_back({ 8.0 });
    return genotype;
  }

  unique_ptr<experimental::replicators::Genotype> suppressed() {
    auto genotype = make_unique<Genotype>();

    auto seg2 = genotype->newSegment();
    seg2->slices[0].appendage = genotype->newSegment();
    seg2->suppressed = true;

    auto seg1 = genotype->newSegment();
    seg1->slices[0].appendage = seg2;

    genotype->root()->slices[0].appendage = seg1;
    return genotype;
  }

  unique_ptr<experimental::replicators::Genotype> appendages() {
    auto genotype = make_unique<Genotype>();
    auto appendage = genotype->newSegment(2.0, 0.1);
    auto appendage2 = genotype->newSegment(1.0, 0.8, appendage);
    genotype->root()->slices.front().appendage = appendage2;
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

    auto level4 = genotype->newSegment(2.0, 1.0);

    auto sss = genotype->newSegment(3.0, 0.5);
    sss->slices.push_back({ 3.0, nullptr });

    auto ss_level2 = genotype->newSegment(1.0, 0.5);
    ss_level2->side_appendage = sss;

    auto ss = genotype->newSegment(3.0, 0.5);
    ss->slices.push_back({ 3.0, ss_level2 });

    auto side_level3 = genotype->newSegment(1.0, 0.2);
    side_level3->side_appendage = ss;

    auto side_level2 = genotype->newSegment(2.0, 2.0);
    side_level2->slices.push_back({ 3.0, side_level3 });
    side_level2->side_appendage = ss;

    auto side_level1 = genotype->newSegment(2.0, 0.5);
    side_level1->slices.push_back({ 3.0, side_level2 });

    auto level3 = genotype->newSegment(0.5, 0.5);
    level3->slices.push_back({ 3.0, level4 });
    level3->side_appendage = side_level1;

    auto level2 = genotype->newSegment(1.0, 1.0);
    level2->slices.push_back({ 3.0, level3 });

    auto level1 = genotype->newSegment(0.2, 1.0);
    level1->slices.push_back({ 3.0, level2 });

    genotype->root()->side_appendage = level1;

    return genotype;
  }

  unique_ptr<experimental::replicators::Genotype> jsonDefinition() {
    auto genotype = make_unique<Genotype>();
    genotype->load(R"({
        "root": "root",
        "segments": [
          {
            "id": "root",
            "length": 1.0,
            "width": 1.0,
            "suppressed": false,
            "side_appendage":
              {
                "length": 4.0,
                "width": 0.1,
                "suppressed": false,
                "side_appendage": null,
                "slices": [ { "relative_width": 1.0, "appendage": null } ]
              },
            "slices": [
              {
                "relative_width": 1.0,
                "appendage":
                  {
                    "length": 2.0,
                    "width": 0.5,
                    "suppressed": false,
                    "side_appendage": null,
                    "slices": [ { "relative_width": 1.0, "appendage": null } ]
                  }
              }
            ]
          }
        ]
      })"_json);
    return genotype;
  }

  unique_ptr<experimental::replicators::Genotype> fingers() {
    auto genotype = make_unique<Genotype>();
    auto nail = genotype->newSegment(3.0, 0.01);
    auto foot = genotype->newSegment(2.0, 5.0);
    foot->slices = { { 1.0, nail },
                     { 1.0, nail },
                     { 1.0, nail },
                     { 2.0, genotype->newSegment(1.0, 0.01) } };
    auto leg = genotype->newSegment(0.5, 0.5, genotype->newSegment(4.0, 0.5, foot));
    genotype->root()->side_appendage = leg;
    return genotype;
  }

  unique_ptr<experimental::replicators::Genotype> multipleBodySegments() {
    auto genotype = make_unique<Genotype>();
    auto nail = genotype->newSegment(3.0, 0.01);
    auto foot = genotype->newSegment(2.0, 3.0);
    foot->slices = { { 1.0, nail }, { 1.0, nail }, { 1.0, nail } };
    auto leg = genotype->newSegment(0.5, 0.1, genotype->newSegment(6.0, 0.5, foot));

    Segment* next_seg = nullptr;
    for (int i = 0; i < 5; ++i) {
      auto body_seg = genotype->newSegment(1.0, 1.1);
      body_seg->slices = { { 1.0, next_seg } };
      body_seg->side_appendage = leg;
      next_seg = body_seg;
    }

    genotype->root()->slices = { { 1.0, next_seg } };
    return genotype;
  }

  unique_ptr<experimental::replicators::Genotype> axialSplit() {
    auto genotype = make_unique<Genotype>();
    auto mid_seg = genotype->newSegment(2.0, 1.5, genotype->newSegment(0.5, 0.001));
    mid_seg->side_appendage =
        genotype->newSegment(0.2, 0.4, genotype->newSegment(2.0, 0.1));
    genotype->root()->slices = { { 1.0, mid_seg } };
    for (int i = 0; i < 3; ++i) {
      genotype->axialSplit(mid_seg, 0.6);
    }
    return genotype;
  }
};

GLOBAL_INITIALIZER {
  registry()->add<Factory>("Segment Tree");
}

Phenotype::Phenotype(const Genotype* genotype) {
  try {
    root_ = createSegment(genotype->root(), nullptr, b2Vec2(), b2Vec2(), false);
  } catch (const std::exception& e) {
    // Failed to generate Phenotype (genotype is not viable)
    // (create a dummy placeholder)
    b2BodyDef body_def;
    b2Body* body = world_.CreateBody(&body_def);

    b2CircleShape shape;
    shape.m_radius = 1.0;

    b2FixtureDef fixture_def;
    fixture_def.shape = &shape;
    body->CreateFixture(&fixture_def);
  }
}

void Phenotype::animateJoint(const Joint& joint, float phase) {
  if (const auto b2_joint = joint.box2d_joint) {
    auto target_speed = cos(phase) * kJointSpeed * (joint.mirror ? -1 : 1);
    if (b2_joint->GetJointAngle() > 0) {
      target_speed -= kJointResetSpeed;
    } else {
      target_speed += kJointResetSpeed;
    }
    b2_joint->SetMotorSpeed(target_speed);
  }

  for (const auto& child_joint : joint.children) {
    animateJoint(child_joint, phase + kPhaseLag);
  }
}

void Phenotype::animate() {
  animateJoint(root_, current_phase_);
  current_phase_ += kPhaseVelocity;

  experimental::replicators::Phenotype::animate();
}

Phenotype::Joint Phenotype::createSegment(const Segment* segment,
                                          b2Body* parent_body,
                                          const b2Vec2& base_left,
                                          const b2Vec2& base_right,
                                          bool mirror) {
  CHECK(segment != nullptr);
  CHECK(!segment->slices.empty());

  Joint joint;
  joint.mirror = mirror;

  if (segment->suppressed) {
    return joint;
  }

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

  // adjust extremity points:
  // 1. fixed length from the base counterpart
  // 2. y-coordinate mirroring
  for (size_t i = 0; i < extremity_points.size(); ++i) {
    const auto& bp = base_points[i];
    auto& ep = extremity_points[i];
    ep = (ep - bp).Normalized() * length + bp;
    ep.y = -ep.y;
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
    shape.m_radius = 0;

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
      joint.children.push_back(createSegment(slice.appendage, body, left, right, mirror));
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
    joint.children.push_back(
        createSegment(segment->side_appendage, body, la_left, la_right, mirror));

    // right appendage (mirror)
    auto ra_left = body->GetWorldPoint(extremity_points.back());
    auto ra_right = body->GetWorldPoint(base_points.back());
    if (mirror) {
      swap(ra_left, ra_right);
    }
    joint.children.push_back(
        createSegment(segment->side_appendage, body, ra_left, ra_right, !mirror));
  }

  // parent joint
  if (parent_body) {
    b2RevoluteJointDef hinge_def;
    hinge_def.bodyA = parent_body;
    hinge_def.bodyB = body;
    hinge_def.localAnchorA = parent_body->GetLocalPoint(body_def.position);
    hinge_def.localAnchorB.Set(0, 0);
    hinge_def.lowerAngle = -0.5f * b2_pi;
    hinge_def.upperAngle = 0.5f * b2_pi;
    if (parent_body) {
      hinge_def.referenceAngle = body->GetAngle() - parent_body->GetAngle();
    }
    hinge_def.enableLimit = true;
    hinge_def.maxMotorTorque = 25.0f;
    hinge_def.motorSpeed = 0;
    hinge_def.enableMotor = true;
    joint.box2d_joint = static_cast<b2RevoluteJoint*>(world_.CreateJoint(&hinge_def));
  }

  return joint;
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
  swap(*this, copy);
  return *this;
}

void swap(Genotype& a, Genotype& b) noexcept {
  using std::swap;
  swap(a.segments_, b.segments_);
  swap(a.root_, b.root_);
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

json Genotype::save() const {
  CHECK(root_ != nullptr);
  CHECK(!segments_.empty());

  // count each segment use count
  // (no need to check for nullptr, just let it have an entry in ref_count)
  unordered_map<const Segment*, int> use_count;
  for (const auto& segment : segments_) {
    ++use_count[segment->side_appendage];
    for (const auto& slice : segment->slices) {
      ++use_count[slice.appendage];
    }
  }

  // every segment with a use count != 1 is saved as a top level subtree root
  unordered_map<const Segment*, int> subtree_id;
  int next_id = 1;
  for (const auto& segment : segments_) {
    if (use_count[segment.get()] != 1) {
      subtree_id[segment.get()] = next_id++;
    }
  }

  // serialize to JSON
  json json_obj;

  function<json(const Segment*, bool)> saveSubtree = [&](const Segment* segment,
                                                         bool top_level) {
    const auto it = subtree_id.find(segment);
    if (!top_level && it != subtree_id.end()) {
      return json(it->second);
    } else if (segment) {
      json seg_json;
      CHECK(!segment->slices.empty());
      if (top_level) {
        CHECK(it != subtree_id.end());
        seg_json["id"] = it->second;
      }
      seg_json["length"] = segment->length;
      seg_json["width"] = segment->width;
      seg_json["suppressed"] = segment->suppressed;
      seg_json["side_appendage"] = saveSubtree(segment->side_appendage, false);
      seg_json["slices"] = json::array();
      for (const auto& slice : segment->slices) {
        seg_json["slices"].push_back({
            { "relative_width", slice.relative_width },
            { "appendage", saveSubtree(slice.appendage, false) },
        });
      }
      return seg_json;
    } else {
      CHECK(!top_level);
      return json(nullptr);
    }
  };

  json_obj["root"] = subtree_id.at(root_);
  for (const auto& [segment, id] : subtree_id) {
    json_obj["segments"].push_back(saveSubtree(segment, true));
  }
  return json_obj;
}

void Genotype::load(const json& json_obj) {
  Genotype tmp;

  // reset genotype
  tmp.root_ = nullptr;
  tmp.segments_.clear();

  unordered_map<string, Segment*> id_to_segment;

  // when loading, we treat segment IDs uniformly as strings
  // (in order to allow friendly IDs for hand-crafted Json definitions)
  auto getId = [](const json& value) {
    if (value.is_number_integer()) {
      return to_string(value.get<int>());
    } else if (value.is_string()) {
      return value.get<string>();
    } else {
      throw core::Exception("Invalid segment ID value");
    }
  };

  function<Segment*(const json&)> loadSegment = [&](const json& seg_json) -> Segment* {
    if (seg_json.is_null()) {
      return nullptr;
    } else if (seg_json.is_object()) {
      Segment* segment = nullptr;

      if (seg_json.contains("id")) {
        const auto segment_id = getId(seg_json.at("id"));
        const auto it = id_to_segment.find(segment_id);
        if (it != id_to_segment.end()) {
          segment = it->second;
          if (!segment->slices.empty()) {
            throw core::Exception("Duplicate segment definition");
          }
        } else {
          segment = tmp.newSegment();
          segment->slices.clear();
          id_to_segment.insert({ segment_id, segment });
        }
      } else {
        segment = tmp.newSegment();
        segment->slices.clear();
      }

      CHECK(segment != nullptr);
      CHECK(segment->slices.empty());

      segment->length = seg_json.at("length");
      segment->width = seg_json.at("width");
      segment->suppressed = seg_json.at("suppressed");
      segment->side_appendage = loadSegment(seg_json.at("side_appendage"));
      for (const auto& slice_json : seg_json.at("slices")) {
        Slice slice;
        slice.relative_width = slice_json.at("relative_width");
        slice.appendage = loadSegment(slice_json.at("appendage"));
        segment->slices.push_back(slice);
      }
      if (segment->slices.empty()) {
        throw core::Exception("Empty segment slices list");
      }

      return segment;
    } else {
      const auto segment_id = getId(seg_json);
      const auto it = id_to_segment.find(segment_id);
      if (it != id_to_segment.end()) {
        return it->second;
      } else {
        // allocate a placeholder segment
        // (placeholder segments have an empty slices array)
        const auto placeholder = tmp.newSegment();
        placeholder->slices.clear();
        id_to_segment.insert({ segment_id, placeholder });
        return placeholder;
      }
    }
  };

  for (const auto& seg_json : json_obj.at("segments")) {
    if (!seg_json.is_object() || !seg_json.contains("id")) {
      throw core::Exception("Invalid top level segment");
    }
    loadSegment(seg_json);
  }

  // check that there are no unresolved ids
  for (const auto& segment : tmp.segments_) {
    CHECK(!segment->slices.empty());
  }

  tmp.root_ = id_to_segment.at(getId(json_obj.at("root")));

  swap(*this, tmp);
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
          stack.push_back(slice.appendage);
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
  slice.appendage = newSegment(1.0, 1.0, slice.appendage);
}

void Genotype::growSideAppendage(Segment* segment) {
  segment->side_appendage = newSegment(1.0, 1.0, segment->side_appendage);
}

// pick a random slice and split it
// (replacing it with two slices with width = old_width * fraction)
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
    slice.appendage = newSegment(new_length, segment->width, slice.appendage);
  }
  if (segment->slices.size() == 1) {
    segment->slices.front().appendage->side_appendage = deepCopy(segment->side_appendage);
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

bool Genotype::operator==(const Genotype& other) const {
  unordered_map<const Segment*, int> this_ids;
  unordered_map<const Segment*, int> other_ids;
  int this_next_id = 1;
  int other_next_id = 1;

  CHECK(root_ != nullptr);
  CHECK(other.root_ != nullptr);

  function<bool(const Segment*, const Segment*)> compareSegments =
      [&](const Segment* this_seg, const Segment* other_seg) {
        if ((this_seg == nullptr) != (other_seg == nullptr)) {
          return false;
        } else if (this_seg == nullptr) {
          return true;
        }

        CHECK(!this_seg->slices.empty());
        CHECK(!other_seg->slices.empty());

        const auto this_it = this_ids.find(this_seg);
        const auto other_it = other_ids.find(other_seg);
        if (this_it == this_ids.end()) {
          if (other_it != other_ids.end()) {
            return false;
          }
          this_ids[this_seg] = this_next_id++;
          other_ids[other_seg] = other_next_id++;
        } else {
          if (other_it == other_ids.end() || other_it->second != this_it->second) {
            return false;
          }
        }

        if (this_seg->slices.size() != other_seg->slices.size()) {
          return false;
        }

        for (size_t i = 0; i < this_seg->slices.size(); ++i) {
          const auto& this_slice = this_seg->slices[i];
          const auto& other_slice = other_seg->slices[i];
          if (this_slice.relative_width != other_slice.relative_width) {
            return false;
          }
          if (!compareSegments(this_slice.appendage, other_slice.appendage)) {
            return false;
          }
        }

        if (this_seg->length != other_seg->length) {
          return false;
        }
        if (this_seg->width != other_seg->width) {
          return false;
        }
        if (this_seg->suppressed != other_seg->suppressed) {
          return false;
        }
        if (!compareSegments(this_seg->side_appendage, other_seg->side_appendage)) {
          return false;
        }

        return true;
      };

  return compareSegments(root_, other.root_);
}

unique_ptr<experimental::replicators::Genotype> Genotype::clone() const {
  return make_unique<Genotype>(*this);
}

}  // namespace experimental::replicators::seg_tree
