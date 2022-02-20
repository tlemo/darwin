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

#include <vector>
#include <utility>
#include <memory>
using namespace std;

namespace experimental::replicators::rigid_cells {

struct Segment;

struct Slice {
  // width relative to the sibling slices
  double relative_width = 1.0;

  // optional appendage attached to the slice's end
  Segment* appendage = nullptr;
};

struct Segment {
  double length = 1.0;
  double width = 1.0;
  bool suppressed = false;
  Segment* side_appendage = nullptr;
  vector<Slice> slices = { Slice() };

  Segment() = default;

  Segment(double length, double width, Segment* appendage = nullptr)
      : length(length), width(width), slices({ { 1.0, appendage } }) {}
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

  Segment* root() const { return root_; }

  template <class... Args>
  Segment* newSegment(Args&&... args) {
    const auto segment = new Segment(std::forward<Args>(args)...);
    segments_.emplace_back(segment);
    return segment;
  }

  // mutations
  void growAppendage(Segment* segment);
  void growSideAppendage(Segment* segment);
  void lateralSplit(Segment* segment, double fraction);
  void axialSplit(Segment* segment, double fraction);
  void mutateLength(Segment* segment, double std_dev);
  void mutateWidth(Segment* segment, double std_dev);
  void mutateSliceWidth(Segment* segment, double std_dev);

 private:
  // deep copy a DAG of segments
  // source segment may be from a different Genotype instance
  Segment* deepCopy(const Segment* segment);

 private:
  // the Genotype owns all the Segment instances
  vector<unique_ptr<Segment>> segments_;

  Segment* root_ = nullptr;
};

class Phenotype : public experimental::replicators::Phenotype {
  static constexpr float kPhaseVelocity = b2_pi / 64;
  static constexpr float kPhaseLag = b2_pi / 4;
  static constexpr float kJointSpeed = 0.15f;
  static constexpr float kJointResetSpeed = 0.05f;

  struct Joint {
    b2RevoluteJoint* box2d_joint = nullptr;
    bool mirror = false;
    vector<Joint> children;
  };

 public:
  explicit Phenotype(const Genotype* genotype);

  void animate() override;

 private:
  Joint createSegment(const Segment* segment,
                      b2Body* parent_body,
                      const b2Vec2& base_left,
                      const b2Vec2& base_right,
                      bool mirror);

  void animateJoint(const Joint& joint, float phase);

 private:
  b2Body* body_ = nullptr;
  b2Fixture* fixture_ = nullptr;

  Joint root_;
  float current_phase_ = 0;
};

}  // namespace experimental::replicators::rigid_cells
