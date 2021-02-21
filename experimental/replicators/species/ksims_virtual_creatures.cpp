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
#include <cmath>
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
    samples.push_back(fish());
    samples.push_back(slug());
    samples.push_back(shell());
    samples.push_back(centipede());
    samples.push_back(spider());
    return samples;
  }

  void runTests() override {
    // TODO
    return;

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

    auto phalanx_2 = genotype->newNode(0.05, 0.1);
    auto phalanx_1 = genotype->newNode(0.05, 0.3);
    auto thumb = genotype->newNode(0.05, 0.2);
    auto palm = genotype->newNode(0.4, 0.2);

    auto forearm = genotype->newNode(0.1, 0.8);
    forearm->recursive_limit = 2;

    auto arm = genotype->newNode(0.3, 0.5);
    arm->recursive_limit = 2;

    auto c0 = genotype->newConnection(genotype->root(), arm, -math::kPi / 3);
    c0->orientation = -math::kPi / 10;
    c0->reflection = true;

    auto c1 = genotype->newConnection(genotype->root(), arm, -2 * math::kPi / 3);
    c1->orientation = -math::kPi / 10;
    c1->reflection = true;

    auto c2 = genotype->newConnection(arm, forearm, 0.0);
    c2->orientation = math::kPi / 5;

    auto c3 = genotype->newConnection(forearm, arm, 0.0);
    c3->orientation = -math::kPi / 5;
    c3->scale = 0.4;

    auto c4 = genotype->newConnection(forearm, palm, 0.0);
    c4->terminal_only = true;

    auto c5 = genotype->newConnection(palm, thumb, math::kPi / 2);
    c5->reflection = true;

    auto c6 = genotype->newConnection(thumb, phalanx_1, -math::kPi / 4);
    c6->orientation = -math::kPi / 2;

    auto c7 = genotype->newConnection(phalanx_1, phalanx_2, math::kPi / 4);
    c7->reflection = true;

    return genotype;
  }

  unique_ptr<experimental::replicators::Genotype> fish() {
    auto genotype = make_unique<Genotype>();

    auto tail_fin = genotype->newNode(0.5, 5.0);
    auto fin = genotype->newNode(0.1, 0.5);

    auto tail_segment = genotype->newNode(1, 0.8);
    tail_segment->recursive_limit = 8;

    auto root = genotype->root();
    root->width = 1.0;
    root->height = 1.0;

    auto c0 = genotype->newConnection(root, fin, math::kPi / 2);
    c0->orientation = math::kPi / 8;
    c0->reflection = true;

    auto c1 = genotype->newConnection(root, tail_segment, math::kPi);
    c1->scale = 0.9;

    auto c2 = genotype->newConnection(tail_segment, tail_segment, 0);
    c2->scale = 0.8;

    auto c3 = genotype->newConnection(tail_segment, tail_fin, 0);
    c3->terminal_only = true;
    c3->orientation = math::kPi / 8;

    return genotype;
  }

  unique_ptr<experimental::replicators::Genotype> slug() {
    auto genotype = make_unique<Genotype>();

    auto bulb = genotype->newNode(0.2, 0.2);
    auto antenna = genotype->newNode(0.1, 2.0);

    auto fore_segment = genotype->newNode(1.0, 0.5);
    fore_segment->recursive_limit = 6;

    auto aft_segment = genotype->newNode(1.0, 0.5);
    aft_segment->recursive_limit = 6;

    auto root = genotype->root();
    root->width = 0.9;
    root->height = 0.9;

    genotype->newConnection(root, fore_segment, math::kPi);
    genotype->newConnection(antenna, bulb, 0);

    auto c1 = genotype->newConnection(fore_segment, fore_segment, 0);
    c1->scale = 1.3;

    auto c2 = genotype->newConnection(root, antenna, math::kPi / 4);
    c2->reflection = true;

    auto c3 = genotype->newConnection(fore_segment, aft_segment, 0);
    c3->terminal_only = true;

    auto c4 = genotype->newConnection(aft_segment, aft_segment, 0);
    c4->scale = 0.7;

    return genotype;
  }

  unique_ptr<experimental::replicators::Genotype> shell() {
    auto genotype = make_unique<Genotype>();

    auto root = genotype->root();
    root->width = 0.1;
    root->height = 0.1;
    root->recursive_limit = 20;
    root->rigid_joint = true;

    auto c0 = genotype->newConnection(root, root, -math::kPi / 6);
    c0->orientation = -math::kPi / 8;
    c0->scale = 1.2;

    return genotype;
  }

  unique_ptr<experimental::replicators::Genotype> centipede() {
    auto genotype = make_unique<Genotype>();

    auto foot = genotype->newNode(0.2, 0.3);
    auto leg = genotype->newNode(0.1, 0.8);

    auto root = genotype->root();
    root->width = 1.0;
    root->height = 1.0;
    root->recursive_limit = 10;

    auto c1 = genotype->newConnection(root, leg, math::kPi / 2);
    c1->orientation = -math::kPi / 8;
    c1->reflection = true;

    genotype->newConnection(leg, foot, math::kPi / 4);
    genotype->newConnection(root, root, 0);

    return genotype;
  }

  unique_ptr<experimental::replicators::Genotype> spider() {
    auto genotype = make_unique<Genotype>();

    auto leg = genotype->newNode(0.1, 0.8);

    auto root = genotype->root();
    root->width = 1.0;
    root->height = 1.0;

    auto c0 = genotype->newConnection(leg, leg, 0);
    c0->orientation = math::kPi / 6;

    auto c1 = genotype->newConnection(root, leg, math::kPi / 4);
    c1->reflection = true;

    auto c2 = genotype->newConnection(root, leg, math::kPi / 3);
    c2->reflection = true;

    auto c3 = genotype->newConnection(root, leg, math::kPi / 2);
    c3->reflection = true;

    auto c4 = genotype->newConnection(root, leg, 3 * math::kPi / 4);
    c4->reflection = true;

    return genotype;
  }
};

GLOBAL_INITIALIZER {
  registry()->add<Factory>("Karl Sims's Virtual Creatures");
}

static SegmentFrame childFrame(SegmentFrame parent_frame,
                               b2Body* body,
                               bool mirror,
                               const Connection& connection,
                               const Node& node) {
  CHECK(body != nullptr);

  const auto mirrorAngle = [&](double angle) {
    return mirror != parent_frame.mirror ? -angle : angle;
  };

  const auto pos_angle = mirrorAngle(connection.position) + math::kPi / 2;
  const auto cx = cos(pos_angle);
  const auto cy = sin(pos_angle);
  const auto t = min(1 / abs(cx), 1 / abs(cy));
  const math::Vector2d p(cx * t, cy * t + 1.0);

  const auto scale = parent_frame.scale;
  const auto half_width = node.width * 0.5;
  const auto half_height = node.height * 0.5;
  const auto transf = math::HMatrix2d::rotate(parent_frame.angle) *
                      math::HMatrix2d::scale(scale * half_width, scale * half_height);

  SegmentFrame child_frame;
  child_frame.scale = parent_frame.scale * connection.scale;
  child_frame.angle = parent_frame.angle + mirrorAngle(connection.position) +
                      mirrorAngle(connection.orientation);
  child_frame.origin = parent_frame.origin + transf * p;
  child_frame.parent_body = body;
  child_frame.mirror = (mirror != parent_frame.mirror);
  return child_frame;
}

Phenotype::Phenotype(const Genotype* genotype) {
  const auto& nodes = genotype->nodes();
  const auto& connections = genotype->connections();
  CHECK(!nodes.empty());

  // build the node -> list of connections map
  unordered_map<int, vector<const Connection*>> node_connections;
  for (auto& c : connections) {
    node_connections[c.src].push_back(&c);
  }

  // track the number of instances of a node (in the current stack)
  // so we can handle the `recursive_limit` attribute
  unordered_map<int, int> node_instances;

  struct StackEntry {
    int node = -1;
    SegmentFrame frame;
    size_t next_child = 0;
    bool mirror = false;
    b2Body* body = nullptr;
  };

  // max segment instantiation depth
  constexpr size_t kMaxStackSize = 128;

  vector<StackEntry> stack = { { 0, SegmentFrame() } };

  try {
    while (!stack.empty()) {
      auto& current = stack.back();

      const auto& node = nodes[current.node];

      // create segment's body and joint
      if (current.body == nullptr) {
        CHECK(current.next_child == 0);
        current.body = createSegment(node, current.frame);
        CHECK(current.body != nullptr);
        ++node_instances[current.node];
      }

      const auto it = node_connections.find(current.node);
      if (it == node_connections.end() || current.next_child >= it->second.size()) {
        --node_instances[current.node];
        if (stack.size() == 1) {
          CHECK(root_ == nullptr);
          CHECK(current.body != nullptr);
          root_ = current.body;
        }
        stack.pop_back();
      } else {
        if (stack.size() > kMaxStackSize) {
          throw core::Exception("Phenotype segment stack overflow");
        }

        const auto connection = it->second[current.next_child];
        if (connection->reflection && !current.mirror) {
          // instantiate the mirrored clone
          current.mirror = true;
        } else {
          current.mirror = false;
          ++current.next_child;
        }

        const bool terminal = node_instances[current.node] == node.recursive_limit;
        if (!connection->terminal_only || terminal) {
          if (node_instances[connection->dst] < nodes[connection->dst].recursive_limit) {
            const auto child_frame = childFrame(
                current.frame, current.body, current.mirror, *connection, node);
            stack.push_back({ connection->dst, child_frame });
          }
        }
      }
    }

    // postcondition (sanity check)
    for (const auto [node, count] : node_instances) {
      CHECK(count == 0);
    }
  } catch (const std::exception& e) {
    // Failed to generate Phenotype (genotype is not viable)
    createDummyBody();
  }
}

void Phenotype::animateJoint(b2Body* body, float phase) {
  for (auto e = body->GetJointList(); e != nullptr; e = e->next) {
    if (e->joint->GetBodyB() == body) {
      if (auto revolute_joint = dynamic_cast<b2RevoluteJoint*>(e->joint)) {
        auto target_speed = cos(phase) * kJointSpeed;
        if (revolute_joint->GetJointAngle() > 0) {
          target_speed -= kJointResetSpeed;
        } else {
          target_speed += kJointResetSpeed;
        }
        revolute_joint->SetMotorSpeed(target_speed);
      }
    } else {
      animateJoint(e->joint->GetBodyB(), phase + kPhaseLag);
    }
  }
}

void Phenotype::animate() {
  animateJoint(root_, current_phase_);
  current_phase_ += kPhaseVelocity;

  experimental::replicators::Phenotype::animate();
}

static b2Vec2 toBox2dVec(const math::Vector2d& v) {
  return b2Vec2(float(v.x), float(v.y));
}

b2Body* Phenotype::createSegment(const Node& node, const SegmentFrame& frame) {
  // angle = 0 is "up" (y axis), relative to the parent coord frame
  const math::Vector2d dir(sin(frame.angle), cos(frame.angle));

  const float half_width = node.width * frame.scale * 0.5;
  const float height = node.height * frame.scale;

  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  body_def.position = toBox2dVec(frame.origin);
  body_def.angle = frame.angle;
  b2Body* body = world_.CreateBody(&body_def);

  array<b2Vec2, 4> points;
  points[0] = b2Vec2(-half_width, 0);
  points[1] = b2Vec2(half_width, 0);
  points[2] = b2Vec2(half_width, height);
  points[3] = b2Vec2(-half_width, height);

  b2PolygonShape shape;
  shape.Set(points.data(), points.size());
  shape.m_radius = 0;

  b2FixtureDef fixture_def;
  fixture_def.shape = &shape;
  fixture_def.density = 1.0f;
  fixture_def.material.color = b2Color(0.8f, 0.5f, 0.5f);
  body->CreateFixture(&fixture_def);

  if (frame.parent_body) {
    if (node.rigid_joint) {
      b2WeldJointDef weld_def;
      weld_def.Initialize(frame.parent_body, body, toBox2dVec(frame.origin));
      world_.CreateJoint(&weld_def);
    } else {
      b2RevoluteJointDef hinge_def;
      hinge_def.Initialize(frame.parent_body, body, toBox2dVec(frame.origin));
      hinge_def.lowerAngle = -0.15f * b2_pi;
      hinge_def.upperAngle = 0.15f * b2_pi;
      hinge_def.enableLimit = true;
      hinge_def.maxMotorTorque = 25.0f;
      hinge_def.motorSpeed = 0;
      hinge_def.enableMotor = true;
      world_.CreateJoint(&hinge_def);
    }
  }

  return body;
}

void Phenotype::createDummyBody() {
  b2BodyDef body_def;
  b2Body* body = world_.CreateBody(&body_def);

  b2CircleShape shape;
  shape.m_radius = 1.0;

  b2FixtureDef fixture_def;
  fixture_def.shape = &shape;
  body->CreateFixture(&fixture_def);
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
