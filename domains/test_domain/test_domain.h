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

#pragma once

#include <core/darwin.h>
#include <core/properties.h>

#include <limits>
using namespace std;

namespace test_domain {

//! TestDomain configuration
struct Config : public core::PropertySet {
  PROPERTY(inputs, int, 8, "Number of inputs");
  PROPERTY(outputs, int, 3, "Number of outputs");

  PROPERTY(input_range, float, 10.0f, "The range of (random) input values");
  PROPERTY(output_range,
           float,
           numeric_limits<float>::infinity(),
           "The expected outputs range (checked at runtime)");

  PROPERTY(fitness_mean, float, 0.0f, "The mean of the fitness values distribution");
  PROPERTY(fitness_stddev, float, 1.0f, "The stddev of the fitness values distribution");
  PROPERTY(fitness_resolution, float, 0.01f, "The resolution of the fitness values");

  PROPERTY(eval_steps, int, 500, "Number of eval steps for each genotype");
};

//! Domain: TestDomain
//! 
//! The TestDomain is a dummy domain implementation. It generates random input values 
//! (in the range of `[-input_range, +input_range]`) and checks that the output values
//! from the population's brains fall in `[-output_range, +output_range]`.
//! 
//! The "evaluated" fitness values are also random, using a normal distribution, rounded
//! to multiples of `fitness_resolution`
//! 
class TestDomain : public darwin::Domain {
 public:
  explicit TestDomain(const core::PropertySet& config);

  size_t inputs() const override;
  size_t outputs() const override;

  bool evaluatePopulation(darwin::Population* population) const override;
  
  const Config& config() const { return config_; }

 private:
  Config config_;
};

class Factory : public darwin::DomainFactory {
  unique_ptr<darwin::Domain> create(const core::PropertySet& config) override;
  unique_ptr<core::PropertySet> defaultConfig(darwin::ComplexityHint hint) const override;
};

inline void init() {
  darwin::registry()->domains.add<Factory>("test_domain");
}

}  // namespace test_domain
