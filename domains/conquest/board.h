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

#include <initializer_list>
#include <string>
#include <vector>
using namespace std;

namespace conquest {

enum class BoardConfiguration {
  Triangle,
  Diamond,
  SimplifiedDiamond,
  Hexagon,
  SimplifiedHexagon,
};

inline auto customStringify(core::TypeTag<BoardConfiguration>) {
  static auto stringify = new core::StringifyKnownValues<BoardConfiguration>{
    { BoardConfiguration::Triangle, "triangle" },
    { BoardConfiguration::Diamond, "diamond" },
    { BoardConfiguration::SimplifiedDiamond, "simple diamond" },
    { BoardConfiguration::Hexagon, "hexagon" },
    { BoardConfiguration::SimplifiedHexagon, "simple hexagon" },
  };
  return stringify;
}

// the board configuration
struct Board {
  struct Node {
    int x;
    int y;
  };

  struct Edge {
    int src;
    int dst;
  };

  struct StartNodes {
    int blue_node;
    int red_node;
  };

  struct Arc {
    int src;
    int dst;
    double length;
  };

  vector<Node> nodes;
  vector<Arc> arcs;
  vector<StartNodes> start_nodes;

  Board(initializer_list<Node> nodes,
        initializer_list<Edge> edges,
        initializer_list<StartNodes> start_nodes);

  static const Board* getBoard(BoardConfiguration configuration);
};

}  // namespace conquest
