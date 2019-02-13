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
#include <core/utils.h>
#include <core/darwin.h>
#include <core/properties.h>
#include <core/stringify.h>
#include <core/roulette_selection.h>
#include <core/cgp_islands_selection.h>
#include <core/truncation_selection.h>

#include <memory>
#include <vector>
using namespace std;

namespace classic {

void init();

enum class CrossoverOp {
  Mix,
  Split,
  RowSplit,
  ColSplit,
  RowOrColSplit,
  PrefRowSplit,
  PrefAverage,
  RowMix,
  ColMix,
  RowOrColMix,
  Quadrants,
  BestParent,
  Randomize,
};

inline auto customStringify(core::TypeTag<CrossoverOp>) {
  static auto stringify = new core::StringifyKnownValues<CrossoverOp>{
    { CrossoverOp::Mix, "mix" },
    { CrossoverOp::Split, "split" },
    { CrossoverOp::RowSplit, "row_split" },
    { CrossoverOp::ColSplit, "col_split" },
    { CrossoverOp::RowOrColSplit, "row_or_col_split" },
    { CrossoverOp::PrefRowSplit, "pref_row_split" },
    { CrossoverOp::PrefAverage, "pref_average" },
    { CrossoverOp::RowMix, "row_mix" },
    { CrossoverOp::ColMix, "col_mix" },
    { CrossoverOp::RowOrColMix, "row_or_col_mix" },
    { CrossoverOp::Quadrants, "quadrants" },
    { CrossoverOp::BestParent, "best_parent" },
    { CrossoverOp::Randomize, "randomize" },
  };
  return stringify;
}

enum class MutationOp {
  IndividualCells,
  AllCells,
  RowOrCol,
  Row,
  Col,
  RowAndCol,
  SubRect,
};

inline auto customStringify(core::TypeTag<MutationOp>) {
  static auto stringify = new core::StringifyKnownValues<MutationOp>{
    { MutationOp::IndividualCells, "individual_cells" },
    { MutationOp::AllCells, "all_cells" },
    { MutationOp::RowOrCol, "row_or_col" },
    { MutationOp::Row, "row" },
    { MutationOp::Col, "col" },
    { MutationOp::RowAndCol, "row_and_col" },
    { MutationOp::SubRect, "sub_rect" },
  };
  return stringify;
}

enum class SelectionAlgorithmType {
  RouletteWheel,
  CgpIslands,
  Truncation,
};

inline auto customStringify(core::TypeTag<SelectionAlgorithmType>) {
  static auto stringify = new core::StringifyKnownValues<SelectionAlgorithmType>{
    { SelectionAlgorithmType::RouletteWheel, "roulette_wheel" },
    { SelectionAlgorithmType::CgpIslands, "cgp_islands" },
    { SelectionAlgorithmType::Truncation, "truncation" },
  };
  return stringify;
}

struct SelectionAlgorithmVariant
    : public core::PropertySetVariant<SelectionAlgorithmType> {
  CASE(SelectionAlgorithmType::RouletteWheel,
       roulette_wheel,
       selection::RouletteSelectionConfig);
  CASE(SelectionAlgorithmType::CgpIslands,
       cgp_islands,
       selection::CgpIslandsSelectionConfig);
  CASE(SelectionAlgorithmType::Truncation,
       truncation,
       selection::TrunctationSelectionConfig);
};

struct Config : public core::PropertySet {
  PROPERTY(activation_function,
           ann::ActivationFunction,
           ann::ActivationFunction::Tanh,
           "Main activation function");

  PROPERTY(gate_activation_function,
           ann::ActivationFunction,
           ann::ActivationFunction::Logistic,
           "Activation function used for cell gates (ex. LSTM)");

  // mutation parameters
  PROPERTY(mutation_chance, float, 0.01f, "Mutation chance");

  // the sizes of each hidden layers, if any
  PROPERTY(hidden_layers, vector<size_t>, {}, "Hidden layers (size of each layer)");

  // genetic operators
  PROPERTY(crossover_operator, CrossoverOp, CrossoverOp::Quadrants, "Crossover operator");
  PROPERTY(mutation_operator,
           MutationOp,
           MutationOp::IndividualCells,
           "Mutation operator");

  PROPERTY(normalize_input, bool, false, "Normalize input values");
  PROPERTY(normalize_output, bool, false, "Normalize output values");

  VARIANT(selection_algorithm,
          SelectionAlgorithmVariant,
          SelectionAlgorithmType::RouletteWheel,
          "Selection algorithm");
};

extern Config g_config;

// TODO: design a better interface
extern size_t g_inputs;
extern size_t g_outputs;

// genetic operators
void crossoverOperator(ann::Matrix& child,
                       const ann::Matrix& parent1,
                       const ann::Matrix& parent2,
                       float preference);

void mutationOperator(ann::Matrix& w, float mutation_std_dev);

// base class for fully connected ANN layers
struct AnnLayer {
  vector<float> values;

  AnnLayer(size_t size) : values(size) {}
  virtual ~AnnLayer() = default;

  virtual void evaluate(const vector<float>& inputs) = 0;
  virtual void resetState() = 0;
};

}  // namespace classic
