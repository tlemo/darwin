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

#include "brain.h"

#include <core/ann_activation_functions.h>

#include <cmath>
#include <assert.h>
#include <limits>
using namespace std;

namespace cgp {

Brain::Brain(const Genotype* genotype) : genotype_(genotype) {
  auto domain = genotype_->population()->domain();

  // start with the inputs set
  registers_.resize(domain->inputs());
  
  // stores the output register index if the node was visited, otherwise 0
  vector<IndexType> nodes_map(genotype_->functionGenes().size());

  for (const auto& output_gene : genotype_->outputGenes()) {
    auto register_index = dfsNodeEval(output_gene.connection, nodes_map);
    outputs_map_.push_back(register_index);
  }
}

void Brain::setInput(int index, float value) {
  assert(index >= 0 && index < int(genotype_->population()->domain()->inputs()));
  registers_[index] = value;
}

float Brain::output(int index) const {
  assert(index >= 0 && index < int(outputs_map_.size()));

  // NaNs results are likely in a brain based on a random genotype,
  // so gracefully map NaNs to +infinity (a valid, but unusual output value, which is
  // likely to result in poor fitness - so the corresponding genotype is penalized)
  const float value = registers_[outputs_map_[index]];
  return isnan(value) ? numeric_limits<float>::infinity() : value;
}

// CONSIDER: switch to an explicit stack to avoid stack overflows
IndexType Brain::dfsNodeEval(IndexType node_index, vector<IndexType>& nodes_map) {
  auto domain = genotype_->population()->domain();
  const size_t inputs_count = domain->inputs();

  // input node?
  if (node_index < inputs_count) {
    return node_index;
  }

  const auto function_node_index = IndexType(node_index - inputs_count);
  CHECK(function_node_index < nodes_map.size());
  
  auto register_index = nodes_map[function_node_index];
  if (register_index != 0) {
    CHECK(register_index >= inputs_count);
    CHECK(register_index < registers_.size());
    return register_index;
  }

  // if not yet visited, do a post-order dfs traversal
  const auto& gene = genotype_->functionGenes()[function_node_index];
  Instruction instruction;
  instruction.function = gene.function;
  for (size_t i = 0; i < gene.connections.size(); ++i) {
    instruction.sources[i] = dfsNodeEval(gene.connections[i], nodes_map);
  }
  instruction.dst = IndexType(registers_.size());
  instructions_.push_back(instruction);
  registers_.emplace_back(0.0f);

  nodes_map[function_node_index] = instruction.dst;
  return instruction.dst;
}

void Brain::think() {
  for (size_t instr_index = 0; instr_index < instructions_.size(); ++instr_index) {
    const auto& instr = instructions_[instr_index];
    float& result = registers_[instr.dst];
    const float& first_arg = registers_[instr.sources[0]];
    const float& second_arg = registers_[instr.sources[1]];
    switch (instr.function) {
      case FunctionId::ConstZero:
        result = 0.0f;
        break;
      case FunctionId::ConstOne:
        result = 1.0f;
        break;
      case FunctionId::ConstTwo:
        result = 2.0f;
        break;
      case FunctionId::ConstPi:
        result = 3.141592653589f;
        break;
      case FunctionId::ConstE:
        result = 2.718281828459f;
        break;
      case FunctionId::Identity:
        result = first_arg;
        break;
      case FunctionId::Add:
        result = first_arg + second_arg;
        break;
      case FunctionId::Subtract:
        result = first_arg - second_arg;
        break;
      case FunctionId::Multiply:
        result = first_arg * second_arg;
        break;
      case FunctionId::Divide:
        result = first_arg / second_arg;
        break;
      case FunctionId::Negate:
        result = -first_arg;
        break;
      case FunctionId::Fmod:
        result = fmod(first_arg, second_arg);
        break;
      case FunctionId::Reminder:
        result = remainder(first_arg, second_arg);
        break;
      case FunctionId::Fdim:
        result = fdim(first_arg, second_arg);
        break;
      case FunctionId::Ceil:
        result = ceil(first_arg);
        break;
      case FunctionId::Floor:
        result = floor(first_arg);
        break;
      case FunctionId::Abs:
        result = fabs(first_arg);
        break;
      case FunctionId::Average:
        result = (first_arg + second_arg) / 2;
        break;
      case FunctionId::Min:
        result = fmin(first_arg, second_arg);
        break;
      case FunctionId::Max:
        result = fmax(first_arg, second_arg);
        break;
      case FunctionId::Square:
        result = first_arg * first_arg;
        break;
      case FunctionId::Log:
        result = log(first_arg);
        break;
      case FunctionId::Log2:
        result = log2(first_arg);
        break;
      case FunctionId::Sqrt:
        result = sqrt(first_arg);
        break;
      case FunctionId::Power:
        result = pow(first_arg, second_arg);
        break;
      case FunctionId::Exp:
        result = exp(first_arg);
        break;
      case FunctionId::Exp2:
        result = exp2(first_arg);
        break;
      case FunctionId::Sin:
        result = sin(first_arg);
        break;
      case FunctionId::Cos:
        result = cos(first_arg);
        break;
      case FunctionId::Tan:
        result = tan(first_arg);
        break;
      case FunctionId::Asin:
        result = asin(first_arg);
        break;
      case FunctionId::Acos:
        result = acos(first_arg);
        break;
      case FunctionId::Atan:
        result = atan(first_arg);
        break;
      case FunctionId::Sinh:
        result = sinh(first_arg);
        break;
      case FunctionId::Cosh:
        result = cosh(first_arg);
        break;
      case FunctionId::Tanh:
        result = tanh(first_arg);
        break;
      case FunctionId::AfnIdentity:
        result = ann::afnIdentity(first_arg);
        break;
      case FunctionId::AfnLogistic:
        result = ann::afnLogistic(first_arg);
        break;
      case FunctionId::AfnTanh:
        result = ann::afnTanh(first_arg);
        break;
      case FunctionId::AfnReLU:
        result = ann::afnReLU(first_arg);
        break;
      case FunctionId::AfnNeat:
        result = ann::afnNeat(first_arg);
        break;
      case FunctionId::CmpEq:
        result = first_arg == second_arg;
        break;
      case FunctionId::CmpNe:
        result = first_arg != second_arg;
        break;
      case FunctionId::CmpGt:
        result = first_arg > second_arg;
        break;
      case FunctionId::CmpGe:
        result = first_arg >= second_arg;
        break;
      case FunctionId::CmpLt:
        result = first_arg < second_arg;
        break;
      case FunctionId::CmpLe:
        result = first_arg <= second_arg;
        break;
      case FunctionId::And:
        result = bool(first_arg) && bool(second_arg);
        break;
      case FunctionId::Or:
        result = bool(first_arg) || bool(second_arg);
        break;
      case FunctionId::Not:
        result = !bool(first_arg);
        break;
      case FunctionId::Xor:
        result = bool(first_arg) != bool(second_arg);
        break;
      case FunctionId::IfOrZero:
        result = bool(first_arg) ? second_arg : 0;
        break;
      default:
        FATAL("Unexpected function id");
    }
  }
}

void Brain::resetState() {
  // nothing to do
}

}  // namespace cgp
