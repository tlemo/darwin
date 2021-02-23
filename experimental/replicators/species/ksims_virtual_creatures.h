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

#pragma once

#include "replicators.h"

#include <core/math_2d.h>

#include <vector>
#include <utility>
#include <memory>
using namespace std;

// An attempt to implement a 2D version of Karl Sims's Virtual Creatures morphology
// (https://www.karlsims.com/papers/siggraph94.pdf)

namespace experimental::replicators::ksims {

template <class T>
class ArrayElem {
 public:
  ArrayElem(vector<T>& array, size_t index) : array_(array), index_(index) {
    CHECK(index_ < array_.size());
  }

  int index() const { return int(index_); }

  T& operator*() const noexcept {
    CHECK(index_ < array_.size());
    return array_[index_];
  }

  T* operator->() const noexcept {
    CHECK(index_ < array_.size());
    return &array_[index_];
  }

 private:
  vector<T>& array_;
  const size_t index_;
};

struct Node {
  double width = 1.0;
  double length = 2.0;
  bool rigid_joint = false;
  int recursive_limit = 4;

  Node(double width, double length) : width(width), length(length) {}

  bool operator==(const Node& other) const;
};

struct Connection {
  // indexes into the nodes array
  int src = -1;
  int dst = -1;

  // connection position is defined through an angle around the Node's center
  double position = 0;

  // angle relative to the connection normal
  double orientation = 0;

  // only used when the recursive limit is reached
  bool terminal_only = false;

  // scale (relative to parent node)
  double scale = 1.0;

  // x-axis mirroring clone?
  bool reflection = false;

  Connection(ArrayElem<Node> src, ArrayElem<Node> dst, double position)
      : src(src.index()), dst(dst.index()), position(position) {}

  bool operator==(const Connection& other) const;
};

class Genotype : public experimental::replicators::Genotype {
 public:
  Genotype();

  Genotype(const Genotype& other);
  Genotype& operator=(const Genotype& other);

  friend void swap(Genotype& a, Genotype& b) noexcept;

  unique_ptr<experimental::replicators::Phenotype> grow() const override;
  unique_ptr<experimental::replicators::Genotype> clone() const override;
  void mutate() override;

  json save() const override;
  void load(const json& json_obj) override;

  auto& nodes() { return nodes_; }
  auto& nodes() const { return nodes_; }

  auto root() { return ArrayElem(nodes_, 0); }

  auto& connections() { return connections_; }
  auto& connections() const { return connections_; }

  template <class... Args>
  auto newNode(Args&&... args) {
    nodes_.emplace_back(std::forward<Args>(args)...);
    return ArrayElem(nodes_, nodes_.size() - 1);
  }

  template <class... Args>
  auto newConnection(Args&&... args) {
    connections_.emplace_back(std::forward<Args>(args)...);
    return ArrayElem(connections_, connections_.size() - 1);
  }

  // mutations
  void mutateNodeWidth(double std_dev);
  void mutateNodeLength(double std_dev);
  void mutateNodeJointType();
  void mutateNodeRecursiveLimitUp();
  void mutateNodeRecursiveLimitDown();

  // for testing purposes
  bool operator==(const Genotype& other) const;

 private:
  vector<Node> nodes_;
  vector<Connection> connections_;
};

struct SegmentFrame {
  math::Vector2d origin;
  math::Scalar scale = 1.0;
  math::Scalar angle = 0;
  bool mirror = false;
  b2Body* parent_body = nullptr;
};

class Phenotype : public experimental::replicators::Phenotype {
  static constexpr float kPhaseVelocity = b2_pi / 64;
  static constexpr float kPhaseLag = b2_pi / 4;
  static constexpr float kJointSpeed = 0.15f;
  static constexpr float kJointResetSpeed = 0.05f;

 public:
  explicit Phenotype(const Genotype* genotype);

  void animate() override;

 private:
  b2Body* createSegment(const Node& node, const SegmentFrame& frame);
  void createDummyBody();
  void animateJoint(b2Body* body, float phase);

 private:
  b2Body* root_ = nullptr;
  float current_phase_ = 0;
};

}  // namespace experimental::replicators::ksims
