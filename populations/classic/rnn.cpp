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

#include "rnn.h"
#include "brain.h"

#include <core/ann_dynamic.h>
#include <core/ann_utils.h>

#include <assert.h>

namespace classic {

template <>
unique_ptr<darwin::Brain> rnn::Genotype::grow() const {
  return make_unique<rnn::Brain>(this);
}

namespace rnn {

Gene::Gene(size_t inputs, size_t outputs)
    : feedforward::Gene(inputs, outputs), rw(1, outputs) {}

void Gene::crossover(const Gene& parent1, const Gene& parent2, float preference) {
  feedforward::Gene::crossover(parent1, parent2, preference);
  crossoverOperator(rw, parent1.rw, parent2.rw, preference);
}

void Gene::mutate(float mutation_std_dev) {
  feedforward::Gene::mutate(mutation_std_dev);
  mutationOperator(rw, mutation_std_dev);
}

void Gene::randomize() {
  feedforward::Gene::randomize();
  ann::randomize(rw);
}

void to_json(nlohmann::json& json_obj, const Gene& gene) {
  to_json(json_obj, static_cast<const feedforward::Gene&>(gene));
  json_obj["rw"] = gene.rw;
}

void from_json(const json& json_obj, Gene& gene) {
  from_json(json_obj, static_cast<feedforward::Gene&>(gene));
  gene.rw = json_obj.at("rw");
  if (gene.rw.cols != gene.w.cols || gene.rw.rows != 1)
    throw core::Exception("Can't load gene, inconsistent RNN weights");
}

Layer::Layer(const Gene& gene) : classic::AnnLayer(gene.w.cols), w(gene.w), rw(gene.rw) {
  CHECK(rw.rows == 1);
  CHECK(rw.cols == w.cols);
}

void Layer::evaluate(const vector<float>& inputs) {
  assert(inputs.size() == w.rows - 1);
  assert(values.size() == w.cols);

  const size_t bias_index = w.rows - 1;
  for (size_t i = 0; i < w.cols; ++i) {
    float value = w[bias_index][i] + values[i] * rw[0][i];
    for (size_t j = 0; j < bias_index; ++j)
      value += inputs[j] * w[j][i];
    values[i] = value;
  }
}

void Layer::resetState() {
  ann::reset(values);
}

}  // namespace rnn
}  // namespace classic
