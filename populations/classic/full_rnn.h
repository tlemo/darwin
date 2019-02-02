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

// an RNN with fully connected recurrent connections
// (from a layer to itself)

#pragma once

#include "brain.h"
#include "classic.h"
#include "feedforward.h"
#include "genotype.h"

#include <core/ann_dynamic.h>
#include <core/ann_utils.h>
#include <core/utils.h>
#include <core/darwin.h>

#include <memory>
#include <vector>
using namespace std;

namespace classic {
namespace full_rnn {

struct Gene : public feedforward::Gene {
  // recurrent links
  ann::Matrix rw;

  Gene() = default;
  Gene(size_t inputs, size_t outputs);

  void crossover(const Gene& parent1, const Gene& parent2, float preference);
  void mutate(float mutation_std_dev);
  void randomize();

  friend void to_json(json& json_obj, const Gene& gene);
  friend void from_json(const json& json_obj, Gene& gene);
};

struct Layer : public classic::AnnLayer {
  explicit Layer(const Gene& gene);

  // points directly to the weights in the genotype
  const ann::Matrix& w;
  const ann::Matrix& rw;

  // previous values
  vector<float> prev_values;

  void evaluate(const vector<float>& inputs) override;
  void resetState() override;
};

struct GenotypeTraits {
  using HiddenLayerGene = full_rnn::Gene;
  using OutputLayerGene = full_rnn::Gene;
};

using Genotype = classic::Genotype<GenotypeTraits>;

struct BrainTraits {
  using Genotype = full_rnn::Genotype;
  using HiddenLayer = full_rnn::Layer;
  using OutputLayer = full_rnn::Layer;

  static constexpr bool kNormalizeHiddenLayers = true;
};

using Brain = classic::Brain<BrainTraits>;

}  // namespace full_rnn
}  // namespace classic
