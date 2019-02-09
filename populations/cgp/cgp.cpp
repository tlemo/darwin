// Copyright 2019 The Darwin Neuroevolution Framework Authors.
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

#include "cgp.h"
#include "population.h"

#include <core/darwin.h>

namespace cgp {

class Factory : public darwin::PopulationFactory {
  unique_ptr<darwin::Population> create(const core::PropertySet& config,
                                        const darwin::Domain& domain) override {
    return make_unique<Population>(config, domain);
  }

  unique_ptr<core::PropertySet> defaultConfig(
      darwin::ComplexityHint hint) const override {
    auto config = make_unique<Config>();
    switch (hint) {
      case darwin::ComplexityHint::Minimal:
        config->rows = 3;
        config->columns = 5;
        config->levels_back = 2;
        config->outputs_use_levels_back = false;
        config->fn_basic_constants = true;
        config->fn_transcendental_constants = false;
        config->fn_basic_arithmetic = true;
        config->fn_extra_arithmetic = false;
        config->fn_common_math = true;
        config->fn_extra_math = false;
        config->fn_trigonometric = false;
        config->fn_hyperbolic = false;
        config->fn_ann_activation = false;
        config->fn_comparisons = false;
        config->fn_logic_gates = false;
        config->fn_conditional = false;
        config->evolvable_constants_count = 1;
        break;
      case darwin::ComplexityHint::Balanced:
        break;
      case darwin::ComplexityHint::Extra:
        config->fn_basic_constants = true;
        config->fn_transcendental_constants = true;
        config->fn_basic_arithmetic = true;
        config->fn_extra_arithmetic = true;
        config->fn_common_math = true;
        config->fn_extra_math = true;
        config->fn_trigonometric = true;
        config->fn_hyperbolic = true;
        config->fn_ann_activation = true;
        config->fn_comparisons = true;
        config->fn_logic_gates = true;
        config->fn_conditional = true;
        config->fn_stateful = true;
        config->evolvable_constants_count = 16;
        config->mutation_strategy.selectCase(MutationStrategy::FixedCount);
        config->mutation_strategy.fixed_count.mutation_count = 100;
        config->selection_algorithm.selectCase(SelectionAlgorithmType::CgpIslands);
        config->selection_algorithm.cgp_islands.island_size = 50;
        break;
    }
    return config;
  }
};

void init() {
  darwin::registry()->populations.add<Factory>("cgp");
}

}  // namespace cgp
