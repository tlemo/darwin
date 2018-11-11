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
enum class ActivationFunction {
  Identity,     //!< Identity
  Logistic,     //!< Logistic
  Tanh,         //!< Hyperbolic tangent (tanh)
  ReLU,         //!< ReLU
  Neat,         //!< Neat activation function
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

}  // namespace ann
