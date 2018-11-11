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

#include <core/ann_activation_functions.h>
#include <core/properties.h>

// A minimal implementation of NEAT, as described here:
// http://nn.cs.utexas.edu/downloads/papers/stanley.ec02.pdf
// (with a few tweaks and additions)

namespace neat {

void init();

struct Config : public core::PropertySet {
  PROPERTY(activation_function,
           ann::ActivationFunction,
           ann::ActivationFunction::Neat,
           "Main activation function");

  PROPERTY(gate_activation_function,
           ann::ActivationFunction,
           ann::ActivationFunction::Logistic,
           "Activation function used for cell gates (ex. LSTM)");

  PROPERTY(implicit_bias_links,
           bool,
           true,
           "Use bias nodes, automatically feeding into all non-input nodes");

  PROPERTY(use_lstm_nodes, bool, false, "Use LSTM nodes instead of basic nodes");

  PROPERTY(use_classic_selection, bool, false, "Selection strategy");

  // elite (direct) reproduction
  PROPERTY(elite_percentage, float, 0.1f, "Elite percentage");
  PROPERTY(elite_min_fitness, float, 0.0f, "Elite min fitness");
  PROPERTY(elite_mutation_chance, float, 0.0f, "Elite mutation chance");

  PROPERTY(recurrent_output_nodes,
           bool,
           false,
           "Create recurrent output nodes (recurrent link to themselves)");
  PROPERTY(recurrent_hidden_nodes,
           bool,
           false,
           "Create recurrent hidden nodes (recurrent link to themselves)");

  PROPERTY(preserve_connectivity,
           bool,
           false,
           "Make sure the genotypes produce fully connected networks after crossover");

  PROPERTY(contiguous_species,
           bool,
           true,
           "Contiguous species (NEAT selection strategy)");

  PROPERTY(larva_age,
           int,
           5,
           "Genotypes under larva age (in generations) are protected from replacement");

  PROPERTY(old_age, int, 25, "Age limit until genotypes are protected from replacement");

  PROPERTY(min_viable_fitness,
           float,
           10.0f,
           "Minimum fitness value to be a candidate for direct mutation");

  // genotype compatibility coefficients
  PROPERTY(c1, double, 1.0, "Genotype compatibility C1 coefficient");
  PROPERTY(c2, double, 0.7, "Genotype compatibility C2 coefficient");
  PROPERTY(c3, double, 0.1, "Genotype compatibility C3 coefficient");

  PROPERTY(compatibility_threshold, double, 5.0, "Species distance threshold");

  PROPERTY(min_species_size,
           int,
           20,
           "Species with less members than this value will go extinct");

  // mutation probabilities
  PROPERTY(weight_mutation_chance, float, 0.01f, "Weight mutation chance");
  PROPERTY(new_link_chance, float, 0.02f, "New link chance");
  PROPERTY(new_node_chance, float, 0.001f, "New node chance");

  PROPERTY(uniform_parents_distribution,
           bool,
           false,
           "Prefer any (rather than fitter) parents?");

  PROPERTY(normalize_input, bool, false, "Normalize input values");
  PROPERTY(normalize_output, bool, false, "Normalize output values");
};

// global configuration values
extern Config g_config;

// TODO: design a better interface
extern int g_inputs;
extern int g_outputs;

}  // namespace neat
