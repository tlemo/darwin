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

// An example of a minimal replicator implementation
// (it can be used as a template to quick start a new type of replicators)
//
// The phenotype is a single rectangle with independently evolvable width and height

namespace experimental::replicators::test {

class Genotype : public experimental::replicators::Genotype {
 public:
  friend void swap(Genotype& a, Genotype& b) noexcept;

  unique_ptr<experimental::replicators::Phenotype> grow() const override;
  unique_ptr<experimental::replicators::Genotype> clone() const override;
  void mutate() override;

  json save() const override;
  void load(const json& json_obj) override;

  // mutations
  void mutateWidth(double std_dev);
  void mutateHeight(double std_dev);

  // for testing purposes
  bool operator==(const Genotype& other) const;

  double width() const { return width_; }
  double height() const { return height_; }

  void setWidth(double width) {
    CHECK(width > 0);
    width_ = width;
  }

  void setHeight(double height) {
    CHECK(height > 0);
    height_ = height;
  }

 private:
  double width_ = 1.0;
  double height_ = 1.0;
};

class Phenotype : public experimental::replicators::Phenotype {
 public:
  explicit Phenotype(const Genotype* genotype);

  void animate() override;
};

}  // namespace experimental::replicators::test
