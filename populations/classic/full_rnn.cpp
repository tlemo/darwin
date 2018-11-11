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

#include "full_rnn.h"
#include "brain.h"

namespace classic {

template <>
unique_ptr<darwin::Brain> full_rnn::Genotype::grow() const {
  return make_unique<full_rnn::Brain>(this);
}

namespace full_rnn {

Gene::Gene(size_t inputs, size_t outputs)
    : feedforward::Gene(inputs, outputs), rw(outputs, outputs) {}

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
  if (gene.rw.cols != gene.w.cols || gene.rw.rows != gene.w.cols)
    throw core::Exception("Can't load gene, inconsistent RNN weights");
}

Layer::Layer(const Gene& gene)
    : classic::AnnLayer(gene.w.cols), w(gene.w), rw(gene.rw), prev_values(gene.w.cols) {
  CHECK(rw.rows == rw.cols);
  CHECK(rw.cols == w.cols);
}

void Layer::evaluate(const vector<float>& inputs) {
  std::swap(values, prev_values);

  ann::evaluateLayer(inputs, values, w);

  // evaluate and add the recurrent connections
  for (size_t i = 0; i < rw.cols; ++i) {
    float value = 0;
    for (size_t j = 0; j < rw.rows; ++j)
      value += prev_values[j] * rw[j][i];
    values[i] += value;
  }
}

void Layer::resetState() {
  ann::reset(values);
  ann::reset(prev_values);
}

}  // namespace full_rnn
}  // namespace classic
