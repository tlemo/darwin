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
#include <core/ann_dynamic.h>

namespace classic {

template <class TRAITS>
class Brain : public darwin::Brain {
  using Genotype = typename TRAITS::Genotype;
  using HiddenLayer = typename TRAITS::HiddenLayer;
  using OutputLayer = typename TRAITS::OutputLayer;

 public:
  Brain(const Genotype* genotype) : output_layer_(genotype->output_layer) {
    CHECK(g_inputs > 0);
    inputs_.resize(g_inputs);
    for (const auto& layer : genotype->hidden_layers)
      hidden_layers_.emplace_back(layer);
  }

  void setInput(int index, float value) override { inputs_[index] = value; }

  float output(int index) const override { return output_layer_.values[index]; }

  void think() override {
    if (g_config.normalize_input)
      ann::activateLayer(inputs_);

    vector<float>* prev_layer = &inputs_;

    for (auto& layer : hidden_layers_) {
      layer.evaluate(*prev_layer);

      if (TRAITS::kNormalizeHiddenLayers)
        ann::activateLayer(layer.values);

      prev_layer = &layer.values;
    }

    output_layer_.evaluate(*prev_layer);

    if (g_config.normalize_output)
      ann::activateLayer(output_layer_.values);
  }

  void resetState() override {
    ann::reset(inputs_);
    for (auto& layer : hidden_layers_)
      layer.resetState();
    output_layer_.resetState();
  }

 private:
  vector<float> inputs_;
  vector<HiddenLayer> hidden_layers_;
  OutputLayer output_layer_;
};

}  // namespace classic
