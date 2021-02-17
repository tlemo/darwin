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

#include "test.h"

#include <core/global_initializer.h>
#include <core/exception.h>
#include <core/utils.h>
#include <core/random.h>

#include <functional>
using namespace std;

namespace experimental::replicators::test {

class Factory : public SpeciesFactory {
 public:
  unique_ptr<experimental::replicators::Genotype> primordialGenotype() override {
    return make_unique<Genotype>();
  }

  vector<unique_ptr<experimental::replicators::Genotype>> samples() override {
    vector<unique_ptr<experimental::replicators::Genotype>> samples;
    samples.push_back(primordialGenotype());
    samples.push_back(tallSample());
    samples.push_back(wideSample());
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
      genotype.mutateWidth(1.0);
      genotype.mutateHeight(1.0);
      genotype.grow();
    }
  }

 private:
  unique_ptr<experimental::replicators::Genotype> tallSample() {
    auto genotype = make_unique<Genotype>();
    genotype->setWidth(0.5);
    genotype->setHeight(8.0);
    return genotype;
  }

  unique_ptr<experimental::replicators::Genotype> wideSample() {
    auto genotype = make_unique<Genotype>();
    genotype->setWidth(6.0);
    genotype->setHeight(0.8);
    return genotype;
  }
};

GLOBAL_INITIALIZER {
  registry()->add<Factory>("Test Replicator");
}

Phenotype::Phenotype(const Genotype* genotype) {
  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  b2Body* body = world_.CreateBody(&body_def);

  b2PolygonShape shape;
  shape.SetAsBox(genotype->width() / 2, genotype->height() / 2);

  b2FixtureDef fixture_def;
  fixture_def.shape = &shape;
  fixture_def.density = 1.0f;
  fixture_def.material.color = b2Color(0.8f, 0.5f, 0.5f);
  body->CreateFixture(&fixture_def);
}

void Phenotype::animate() {
  experimental::replicators::Phenotype::animate();
}

void swap(Genotype& a, Genotype& b) noexcept {
  using std::swap;
  swap(a.width_, b.width_);
  swap(a.height_, b.height_);
}

unique_ptr<experimental::replicators::Phenotype> Genotype::grow() const {
  return make_unique<Phenotype>(this);
}

void Genotype::mutate() {
  struct MutationType {
    double weight = 0;
    function<void()> mutate;
  };

  vector<MutationType> mutagen = {
    { 5, [&] { mutateWidth(1.0); } },
    { 5, [&] { mutateHeight(1.0); } },
    { 25, [&] { mutateWidth(0.1); } },
    { 25, [&] { mutateHeight(0.1); } },
  };

  core::randomWeightedElem(mutagen)->mutate();
}

json Genotype::save() const {
  json json_obj;
  json_obj["width"] = width_;
  json_obj["height"] = height_;
  return json_obj;
}

void Genotype::load(const json& json_obj) {
  Genotype tmp;
  tmp.setWidth(json_obj.at("width"));
  tmp.setHeight(json_obj.at("height"));
  swap(*this, tmp);
}

void Genotype::mutateWidth(double std_dev) {
  width_ = core::clampValue(core::mutateNormalValue(width_, std_dev), 0.01, 1000.0);
}

void Genotype::mutateHeight(double std_dev) {
  height_ = core::clampValue(core::mutateNormalValue(height_, std_dev), 0.01, 1000.0);
}

bool Genotype::operator==(const Genotype& other) const {
  return width_ == other.width_ && height_ == other.height_;
}

unique_ptr<experimental::replicators::Genotype> Genotype::clone() const {
  auto clone = make_unique<Genotype>();
  clone->setWidth(width_);
  clone->setHeight(height_);
  return clone;
}

}  // namespace experimental::replicators::test
