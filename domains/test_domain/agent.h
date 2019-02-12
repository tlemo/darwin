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

#include "test_domain.h"

#include <core/darwin.h>

#include <memory>
using namespace std;

namespace test_domain {

class Agent {
 public:
  Agent(const darwin::Genotype* genotype, const TestDomain* domain);

  float evaluate();

 private:
  unique_ptr<darwin::Brain> brain_;
  const TestDomain* domain_ = nullptr;
};

}  // namespace test_domain
