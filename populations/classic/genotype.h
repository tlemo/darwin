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

#pragma once

#include "classic.h"

#include <core/darwin.h>

namespace classic {

template <class TRAITS>
class Genotype : public darwin::Genotype {
  using HiddenLayerGene = typename TRAITS::HiddenLayerGene;
  using OutputLayerGene = typename TRAITS::OutputLayerGene;

 public:
  vector<HiddenLayerGene> hidden_layers;
  OutputLayerGene output_layer;

 public:
  Genotype() {
    CHECK(g_inputs > 0);
    CHECK(g_outputs > 0);
    size_t prev_size = g_inputs;
    for (size_t size : g_config.hidden_layers) {
      hidden_layers.emplace_back(prev_size, size);
      prev_size = size;
    }
    output_layer = { prev_size, g_outputs };
  }

  void reset() override {
    darwin::Genotype::reset();
  }

  unique_ptr<darwin::Genotype> clone() const override {
    return make_unique<Genotype>(*this);
  }

  json save() const override {
    json json_obj;
    json_obj["hidden_layers"] = hidden_layers;
    json_obj["output_layer"] = output_layer;
    return json_obj;
  }

  void load(const json& json_obj) override {
    // load the genotype
    Genotype tmp_genotype;
    tmp_genotype.hidden_layers =
        json_obj.at("hidden_layers").get<vector<HiddenLayerGene>>();
    tmp_genotype.output_layer = json_obj.at("output_layer");

    // validate the genotype topology
    size_t prev_size = g_inputs;
    for (const auto& layer : tmp_genotype.hidden_layers) {
      if (prev_size + 1 != layer.w.rows)
        throw core::Exception("Can't load genotype, invalid topology");
      prev_size = layer.w.cols;
      if (prev_size == 0)
        throw core::Exception("Can't load genotype, invalid topology");
    }
    if (prev_size + 1 != tmp_genotype.output_layer.w.rows)
      throw core::Exception("Can't load genotype, invalid topology");
    if (tmp_genotype.output_layer.w.cols != g_outputs)
      throw core::Exception("Can't load genotype, invalid topology");

    // if everything went well, replace the genotype with the loaded one
    std::swap(*this, tmp_genotype);
  }

  unique_ptr<darwin::Brain> grow() const override;

  void inherit(const Genotype& parent1, const Genotype& parent2, float preference) {
    // hidden layers
    const size_t layers_count = hidden_layers.size();
    CHECK(layers_count == parent1.hidden_layers.size());
    CHECK(layers_count == parent2.hidden_layers.size());
    for (size_t i = 0; i < layers_count; ++i) {
      hidden_layers[i].crossover(
          parent1.hidden_layers[i], parent2.hidden_layers[i], preference);
    }

    // output layer
    output_layer.crossover(parent1.output_layer, parent2.output_layer, preference);
  }

  void mutate() {
    for (auto& layer : hidden_layers) {
      layer.mutate(ann::g_config.mutation_std_dev);
    }
    output_layer.mutate(ann::g_config.mutation_std_dev);
  }

  void createPrimordialSeed() {
    reset();
    for (auto& layer : hidden_layers) {
      layer.randomize();
    }
    output_layer.randomize();
  }
};

}  // namespace classic
