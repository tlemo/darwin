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

// identity function
float afnIdentity(float x) {
  return x;
}

// standard logistic function
float afnLogistic(float x) {
  return 1 / (1 + exp(-x));
}

// tanh
float afnTanh(float x) {
  return tanh(x);
}

// ReLU
float afnReLU(float x) {
  return x > 0 ? x : 0;
}

// classic NEAT activation function
float afnNeat(float x) {
  constexpr float kSlope = 4.924273f;  // NEAT magic constant
  return 1 / (1 + exp(-x * kSlope));
}

// experimental: ReExp
// TODO: review/evaluate
float afnReExp(float x) {
  return x > 0 ? (1 - exp(-x)) : 0;
}

// experimental
// TODO: review/evaluate
float afnLogisticEx(float x) {
  return 1 / (1 + exp(-2 * (x - 2)));
}

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
