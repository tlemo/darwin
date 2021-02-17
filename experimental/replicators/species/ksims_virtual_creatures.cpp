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

#include "ksims_virtual_creatures.h"

#include <core/global_initializer.h>
#include <core/exception.h>
#include <core/utils.h>
#include <core/math_2d.h>
#include <core/random.h>

#include <unordered_map>
#include <functional>
#include <array>
#include <string>
using namespace std;

namespace experimental::replicators::ksims {

bool Node::operator==(const Node& other) const {
  return width == other.width && height == other.height &&
         rigid_joint == other.rigid_joint && recursive_limit == other.recursive_limit;
}

bool Connection::operator==(const Connection& other) const {
  return src == other.src && dst == other.dst && position == other.position &&
         orientation == other.orientation && terminal_only == other.terminal_only &&
         scale == other.scale && reflection == other.reflection;
}

class Factory : public SpeciesFactory {
 public:
  unique_ptr<experimental::replicators::Genotype> primordialGenotype() override {
    return make_unique<Genotype>();
  }

  vector<unique_ptr<experimental::replicators::Genotype>> samples() override {
    vector<unique_ptr<experimental::replicators::Genotype>> samples;
    samples.push_back(primordialGenotype());
    samples.push_back(mirrorSideLimb());
    return samples;
  }

  void runTests() override {
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
  unique_ptr<experimental::replicators::Genotype> mirrorSideLimb() {
    auto genotype = make_unique<Genotype>();

    auto palm = genotype->newNode(0.5, 0.3);

    auto forearm = genotype->newNode(0.2, 1.5);
    forearm->recursive_limit = 2;

    auto arm = genotype->newNode(0.4, 1.2);
    arm->recursive_limit = 2;

    genotype->newConnection(genotype->root(), arm, -math::kPi / 2);

    auto c1 = genotype->newConnection(genotype->root(), arm, math::kPi / 2);
    c1->reflection = true;

    genotype->newConnection(arm, forearm, 0.0);
    genotype->newConnection(forearm, arm, 0.0);

    auto c2 = genotype->newConnection(forearm, palm, 0.0);
    c2->terminal_only = true;

    return genotype;
  }
};

GLOBAL_INITIALIZER {
  registry()->add<Factory>("Karl Sims's Virtual Creatures");
}

Phenotype::Phenotype(const Genotype* genotype) {
  CHECK(!genotype->nodes().empty());
  try {
    // TODO
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

Genotype::Genotype() {
  newNode(0.5, 2.0);
}

Genotype::Genotype(const Genotype& other)
    : nodes_(other.nodes_), connections_(other.connections_) {}

Genotype& Genotype::operator=(const Genotype& other) {
  Genotype copy(other);
  swap(*this, copy);
  return *this;
}

void swap(Genotype& a, Genotype& b) noexcept {
  using std::swap;
  swap(a.nodes_, b.nodes_);
  swap(a.connections_, b.connections_);
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
  return nodes_ == other.nodes_ && connections_ == other.connections_;
}

unique_ptr<experimental::replicators::Genotype> Genotype::clone() const {
  return make_unique<Genotype>(*this);
}

}  // namespace experimental::replicators::ksims
