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

#include <core/stringify.h>

#include <cmath>
using namespace std;

namespace ann {

//! The types of supported activation functions
//! 
//! \note Consider adding new activation functions to cgp::FunctionId as well
//! 
enum class ActivationFunction {
  Identity,     //!< Identity
  Logistic,     //!< Logistic
  Tanh,         //!< Hyperbolic tangent (tanh)
  ReLU,         //!< ReLU
  Neat,         //!< NEAT activation function
  ReExp,        //!< Experimental
  LogisticEx    //!< Experimental
};

inline auto customStringify(core::TypeTag<ActivationFunction>) {
  static auto stringify = new core::StringifyKnownValues<ActivationFunction>{
    { ActivationFunction::Identity, "identity" },
    { ActivationFunction::Logistic, "logistic" },
    { ActivationFunction::Tanh, "tanh" },
    { ActivationFunction::ReLU, "relu" },
    { ActivationFunction::Neat, "neat" },
    { ActivationFunction::ReExp, "re_exp" },
    { ActivationFunction::LogisticEx, "logistic_ex" },
  };
  return stringify;
}

using ActivationFunctionPfn = float (*)(float);

extern ActivationFunctionPfn g_activation_function;
extern ActivationFunctionPfn g_gate_activation_function;

//! Selects the activation function
void setActivationFunction(ActivationFunction afn);

//! Selects the gate activation function
//! (used with ANNs which include gates, for example LSTM)
void setGateActivationFunction(ActivationFunction afn);

//! Applies the selected activation function
//! \sa setActivationFunction
inline float activate(float x) {
  return (*g_activation_function)(x);
}

//! Applies the selected gate activation function
//! \sa setGateActivationFunction
inline float activateGate(float x) {
  return (*g_gate_activation_function)(x);
}

//! Identity function
inline float afnIdentity(float x) {
  return x;
}

//! Standard logistic function
inline float afnLogistic(float x) {
  return 1 / (1 + exp(-x));
}

//! tanh
inline float afnTanh(float x) {
  return tanh(x);
}

//! ReLU
inline float afnReLU(float x) {
  return x > 0 ? x : 0;
}

//! Classic NEAT activation function
inline float afnNeat(float x) {
  constexpr float kSlope = 4.924273f;  // NEAT magic constant
  return 1 / (1 + exp(-x * kSlope));
}

//! Experimental: ReExp
//! \todo review/evaluate
inline float afnReExp(float x) {
  return x > 0 ? (1 - exp(-x)) : 0;
}

//! Experimental: Yet another sigmoid function
//! \todo review/evaluate
inline float afnLogisticEx(float x) {
  return 1 / (1 + exp(-2 * (x - 2)));
}

}  // namespace ann
