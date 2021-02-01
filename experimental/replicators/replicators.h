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

#include <core/utils.h>
#include <core/modules.h>
#include <third_party/box2d/box2d.h>

#include <third_party/json/json.h>
using nlohmann::json;

#include <memory>
#include <vector>
using namespace std;

namespace experimental::replicators {

class Phenotype : public core::NonCopyable {
 public:
  Phenotype();
  virtual ~Phenotype() = default;

  //! The returned b2World is owned by the Phenotype instance
  virtual b2World* specimen() { return &world_; }

  //! Animate the phenotype (a single step)
  virtual void animate() {}

 protected:
  b2World world_;
};

class Genotype : public core::NonCopyable {
 public:
  virtual ~Genotype() = default;

  virtual unique_ptr<Phenotype> grow() const = 0;
  virtual unique_ptr<Genotype> clone() const = 0;
  virtual void mutate() = 0;

  virtual json save() const = 0;
  virtual void load(const json& json_obj) = 0;
};

class SpeciesFactory : public core::ModuleFactory {
 public:
  virtual unique_ptr<Genotype> primordialGenotype() = 0;

  //! Returns a list of illustrative sample genotypes
  //! (this allows for basic 'guru-checks-output' type of testing)
  virtual vector<unique_ptr<Genotype>> samples() = 0;
};

inline auto registry() {
  static core::ImplementationsSet<SpeciesFactory> instance;
  return &instance;
}

}  // namespace experimental::replicators
