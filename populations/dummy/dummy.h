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

#include <core/properties.h>

namespace dummy {

void init();

struct Config : public core::PropertySet {
  PROPERTY(input_range, float, 10.0f, "The expected inputs range (checked at runtime)");
  PROPERTY(output_range, float, 10.0f, "The range of (random) output values");
  PROPERTY(random_outputs, bool, true, "Generate random outputs");
  PROPERTY(const_output, float, 0.0f, "Used for all outputs if random_outputs is false");
};

}  // namespace dummy
