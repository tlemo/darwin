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

#include "dummy_domain.h"

#include <core/utils.h>
#include <populations/cne/cne.h>
#include <populations/cne/feedforward.h>
#include <populations/cne/full_rnn.h>
#include <populations/cne/lstm.h>
#include <populations/cne/lstm_lite.h>
#include <populations/cne/rnn.h>

#include <tests/testcase_output.h>
#include <third_party/gtest/gtest.h>

#include <memory>
using namespace std;

namespace cne_genotypes_tests {

struct TestParam {
  size_t inputs = 0;
  size_t outputs = 0;
  shared_ptr<cne::Config> population_config = make_shared<cne::Config>();
};

struct CneGenotypesTest : public testing::TestWithParam<TestParam> {
  void initializePopulation(const string& population_name) {
    CHECK(!domain);
    CHECK(!population);

    const auto& test_param = GetParam();

    auto factory = darwin::registry()->populations.find(population_name);
    CHECK(factory != nullptr);

    domain = make_unique<DummyDomain>(test_param.inputs, test_param.outputs);

    // create a dummy population to make sure the population state is initialized
    population = factory->create(*test_param.population_config, *domain);
    CHECK(population);
  }

  unique_ptr<DummyDomain> domain;
  unique_ptr<darwin::Population> population;
};

template <class GENOTYPE>
void testGenotypeSaveLoad() {
  GENOTYPE src_genotype;
  src_genotype.createPrimordialSeed();
  auto json_src = src_genotype.save();

  GENOTYPE dst_genotype;
  dst_genotype.load(json_src);
  auto json_dst = dst_genotype.save();

  EXPECT_EQ(json_src, json_dst);
}

TEST_P(CneGenotypesTest, FF_Genotype_Roundtrip) {
  initializePopulation("cne.feedforward");
  testGenotypeSaveLoad<cne::feedforward::Genotype>();
}

TEST_P(CneGenotypesTest, Full_RNN_Genotype_Roundtrip) {
  initializePopulation("cne.full_rnn");
  testGenotypeSaveLoad<cne::full_rnn::Genotype>();
}

TEST_P(CneGenotypesTest, LSTM_Genotype_Roundtrip) {
  initializePopulation("cne.lstm");
  testGenotypeSaveLoad<cne::lstm::Genotype>();
}

TEST_P(CneGenotypesTest, LSTM_Lite_Genotype_Roundtrip) {
  initializePopulation("cne.lstm_lite");
  testGenotypeSaveLoad<cne::lstm_lite::Genotype>();
}

TEST_P(CneGenotypesTest, RNN_Genotype_Roundtrip) {
  initializePopulation("cne.rnn");
  testGenotypeSaveLoad<cne::rnn::Genotype>();
}

vector<TestParam> everyTestVariation() {
  vector<TestParam> variations;

  {
    TestParam test_param;
    test_param.inputs = 1;
    test_param.outputs = 1;
    test_param.population_config->hidden_layers = {};
    variations.push_back(test_param);
  }

  {
    TestParam test_param;
    test_param.inputs = 1;
    test_param.outputs = 5;
    test_param.population_config->hidden_layers = {};
    variations.push_back(test_param);
  }

  {
    TestParam test_param;
    test_param.inputs = 5;
    test_param.outputs = 1;
    test_param.population_config->hidden_layers = {};
    variations.push_back(test_param);
  }

  {
    TestParam test_param;
    test_param.inputs = 3;
    test_param.outputs = 3;
    test_param.population_config->hidden_layers = {};
    variations.push_back(test_param);
  }

  {
    TestParam test_param;
    test_param.inputs = 4;
    test_param.outputs = 4;
    test_param.population_config->hidden_layers = { 8 };
    variations.push_back(test_param);
  }

  {
    TestParam test_param;
    test_param.inputs = 1;
    test_param.outputs = 1;
    test_param.population_config->hidden_layers = { 1, 1 };
    variations.push_back(test_param);
  }

  {
    TestParam test_param;
    test_param.inputs = 1;
    test_param.outputs = 1;
    test_param.population_config->hidden_layers = { 16, 1, 7 };
    variations.push_back(test_param);
  }

  return variations;
}

INSTANTIATE_TEST_CASE_P(All,
                        CneGenotypesTest,
                        testing::ValuesIn(everyTestVariation()));

}  // namespace cne_genotypes_tests
