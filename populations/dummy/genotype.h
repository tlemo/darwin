// Copyright 2019 The Darwin Neuroevolution Framework Authors.
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

#include "dummy.h"

#include <core/darwin.h>

#include <random>
using namespace std;

namespace dummy {

class Population;

class Genotype : public darwin::Genotype {
 public:
  explicit Genotype(const Population* population);

  unique_ptr<darwin::Brain> grow() const override;
  unique_ptr<darwin::Genotype> clone() const override;

  json save() const override;
  void load(const json& json_obj) override;
  void reset() override;

  auto population() const { return population_; }
  auto seed() const { return seed_; }

 private:
  const Population* population_ = nullptr;
  random_device::result_type seed_ = 0;
};

}  // namespace dummy
