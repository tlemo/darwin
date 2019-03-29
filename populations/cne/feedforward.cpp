// Copyright 2018 The Darwin Neuroevolution Framework Authors.
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

#include "feedforward.h"
#include "brain.h"

#include <core/ann_dynamic.h>
#include <core/ann_utils.h>

namespace cne {

template <>
unique_ptr<darwin::Brain> feedforward::Genotype::grow() const {
  return make_unique<feedforward::Brain>(this);
}

namespace feedforward {

Gene::Gene(size_t inputs, size_t outputs) : w(inputs + 1, outputs) {}

void Gene::crossover(const Gene& parent1, const Gene& parent2, float preference) {
  crossoverOperator(w, parent1.w, parent2.w, preference);
}

void Gene::mutate(float mutation_std_dev) {
  mutationOperator(w, mutation_std_dev);
}

void Gene::randomize() {
  ann::randomize(w);
}

void to_json(nlohmann::json& json_obj, const Gene& gene) {
  json_obj["w"] = gene.w;
}

void from_json(const json& json_obj, Gene& gene) {
  gene.w = json_obj.at("w");
}

Layer::Layer(const Gene& gene) : cne::AnnLayer(gene.w.cols), w(gene.w) {}

void Layer::evaluate(const vector<float>& inputs) {
  ann::evaluateLayer(inputs, values, w);
}

void Layer::resetState() {
  ann::reset(values);
}

}  // namespace feedforward
}  // namespace cne
