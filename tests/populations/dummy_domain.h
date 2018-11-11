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

#include <core/utils.h>
#include <core/darwin.h>

class DummyDomain : public darwin::Domain {
 public:
  DummyDomain(size_t inputs, size_t outputs) : inputs_(inputs), outputs_(outputs) {}

  virtual size_t inputs() const override { return inputs_; }
  virtual size_t outputs() const override { return outputs_; }

  bool evaluatePopulation(darwin::Population*) const override { return true; }

 private:
  size_t inputs_ = 0;
  size_t outputs_ = 0;
};
