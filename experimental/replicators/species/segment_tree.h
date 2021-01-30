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

namespace experimental::replicators::seg_tree {

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

  explicit Segment(Segment* appendage) : slices({ { 1.0, appendage } }) {}
};

class Genotype : public experimental::replicators::Genotype {
  static constexpr int kMutationCount = 1;

 public:
  Genotype();

  Genotype(const Genotype& other);
  Genotype& operator=(const Genotype& other);

  unique_ptr<experimental::replicators::Phenotype> grow() const override;
  unique_ptr<experimental::replicators::Genotype> clone() const override;
  void mutate() override;

  Segment* root() const { return root_; }

  template <class... Args>
  Segment* newSegment(Args&&... args) {
    const auto segment = new Segment(std::forward<Args>(args)...);
    segments_.emplace_back(segment);
    return segment;
  }

  // deep copy a DAG of segments
  // source segment may be from a different Genotype instance
  Segment* deepCopy(const Segment* segment);

  // mutations
  void growAppendage(Segment* segment);
  void growSideAppendage(Segment* segment);
  void lateralSplit(Segment* segment, double fraction);
  void axialSplit(Segment* segment, double fraction);
  void mutateLength(Segment* segment, double std_dev);
  void mutateWidth(Segment* segment, double std_dev);
  void mutateSliceWidth(Segment* segment, double std_dev);

 private:
  // the Genotype owns all the Segment instances
  vector<unique_ptr<Segment>> segments_;

  Segment* root_ = nullptr;
};

class Phenotype : public experimental::replicators::Phenotype {
 public:
  explicit Phenotype(const Genotype* genotype);

 private:
  void createSegment(const Segment* segment,
                     b2Body* parent_body,
                     const b2Vec2& base_left,
                     const b2Vec2& base_right);
};

}  // namespace experimental::replicators::seg_tree
