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

#include "brain.h"
#include "cne.h"
#include "genotype.h"

#include <core/utils.h>
#include <core/darwin.h>

#include <memory>
#include <vector>
using namespace std;

namespace cne {
namespace feedforward {

struct Gene {
  // the weights of a fully connected layer, w[INPUTS+1][OUTPUTS]
  //  w[i][j]     : connection from in.i -> out.j
  //  w[last][j]  : bias weight for out.j
  ann::Matrix w;

  Gene() = default;
  Gene(size_t inputs, size_t outputs);

  void crossover(const Gene& parent1, const Gene& parent2, float preference);
  void mutate(float mutation_std_dev);
  void randomize();

  friend void to_json(json& json_obj, const Gene& gene);
  friend void from_json(const json& json_obj, Gene& gene);
};

struct Layer : public cne::AnnLayer {
  explicit Layer(const Gene& gene);

  // points directly to the weights in the genotype
  const ann::Matrix& w;

  void evaluate(const vector<float>& inputs) override;
  void resetState() override;
};

struct GenotypeTraits {
  using HiddenLayerGene = feedforward::Gene;
  using OutputLayerGene = feedforward::Gene;
};

using Genotype = cne::Genotype<GenotypeTraits>;

struct BrainTraits {
  using Genotype = feedforward::Genotype;
  using HiddenLayer = feedforward::Layer;
  using OutputLayer = feedforward::Layer;

  static constexpr bool kNormalizeHiddenLayers = true;
};

using Brain = cne::Brain<BrainTraits>;

}  // namespace feedforward
}  // namespace cne
