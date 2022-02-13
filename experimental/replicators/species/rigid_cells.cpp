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

#include "rigid_cells.h"

#include <core/global_initializer.h>
#include <core/exception.h>
#include <core/utils.h>
#include <core/random.h>

#include <unordered_map>
#include <random>
#include <functional>
#include <array>
#include <string>
using namespace std;

namespace experimental::replicators::rigid_cells {

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
    samples.push_back(fingers());
    samples.push_back(multipleBodySegments());
    samples.push_back(axialSplit());
    return samples;
  }

  void runTests() override {
    // generating phenotypes
    for (const auto& sample : samples()) {
      sample->grow();
    }

    // mutations
    for (const auto& sample : samples()) {
      auto& genotype = dynamic_cast<Genotype&>(*sample);
      //genotype.growAppendage(genotype.root());
      //genotype.growSideAppendage(genotype.root());
      //genotype.lateralSplit(genotype.root(), 0.2);
      //genotype.axialSplit(genotype.root(), 0.8);
      //genotype.mutateLength(genotype.root(), 1.0);
      //genotype.mutateWidth(genotype.root(), 1.0);
      //genotype.mutateSliceWidth(genotype.root(), 1.0);
      genotype.grow();
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
  registry()->add<Factory>("Rigid Cells");
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
    fixture_def.material.color = b2Color(0.8f, 0.5f, 0.5f);
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
    hinge_def.referenceAngle = body->GetAngle() - parent_body->GetAngle();
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
  const auto segment = core::randomElem(segments_)->get();

  struct MutationType {
    double weight = 0;
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

  core::randomWeightedElem(mutagen)->mutate();
}

json Genotype::save() const {
  FATAL("Not implemented");
}

void Genotype::load(const json& /*json_obj*/) {
  FATAL("Not implemented");
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
  auto& slice = *core::randomElem(segment->slices);
  slice.appendage = newSegment(1.0, 1.0, slice.appendage);
}

void Genotype::growSideAppendage(Segment* segment) {
  segment->side_appendage = newSegment(1.0, 1.0, segment->side_appendage);
}

// pick a random slice and split it
// (replacing it with two slices with width = old_width * fraction)
void Genotype::lateralSplit(Segment* segment, double fraction) {
  CHECK(fraction > 0);

  const auto slice_it = core::randomElem(segment->slices);
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
  segment->length =
      core::clampValue(core::mutateNormalValue(segment->length, std_dev), 0.01, 1000.0);
}

void Genotype::mutateWidth(Segment* segment, double std_dev) {
  segment->width =
      core::clampValue(core::mutateNormalValue(segment->width, std_dev), 0.01, 1000.0);
}

void Genotype::mutateSliceWidth(Segment* segment, double std_dev) {
  auto& slice = *core::randomElem(segment->slices);
  slice.relative_width = core::clampValue(
      core::mutateNormalValue(slice.relative_width, std_dev), 0.01, 100.0);
}

unique_ptr<experimental::replicators::Genotype> Genotype::clone() const {
  return make_unique<Genotype>(*this);
}

}  // namespace experimental::replicators::rigid_cells
