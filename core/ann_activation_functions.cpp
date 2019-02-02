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

#include "ann_activation_functions.h"

namespace ann {

ActivationFunctionPfn g_activation_function = nullptr;
ActivationFunctionPfn g_gate_activation_function = nullptr;

static ActivationFunctionPfn activationFunctionPfn(ActivationFunction afn) {
  switch (afn) {
    case ActivationFunction::Identity:
      return &afnIdentity;
    case ActivationFunction::Logistic:
      return &afnLogistic;
    case ActivationFunction::Tanh:
      return &afnTanh;
    case ActivationFunction::ReLU:
      return &afnReLU;
    case ActivationFunction::Neat:
      return &afnNeat;
    case ActivationFunction::ReExp:
      return &afnReExp;
    case ActivationFunction::LogisticEx:
      return &afnLogisticEx;
    default:
      FATAL("Unexpected activation function");
  }
}

void setActivationFunction(ActivationFunction afn) {
  g_activation_function = activationFunctionPfn(afn);
}

void setGateActivationFunction(ActivationFunction afn) {
  g_gate_activation_function = activationFunctionPfn(afn);
}

}  // namespace ann
