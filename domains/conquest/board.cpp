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

#include "board.h"

#include <assert.h>
#include <math.h>
using namespace std;

namespace conquest {

const Board triangle = {
  // nodes
  {
      { 0, -100 },
      { -100, 0 },
      { 100, 0 },
  },

  // edges
  {
      { 0, 1 },
      { 0, 2 },
      { 1, 2 },
  },

  // start positions
  {
      { 1, 2 },
  },
};

const Board diamond = {
  // nodes
  {
      { 0, 0 },
      { -100, 0 },
      { 0, 100 },
      { 100, 0 },
      { 0, -100 },
  },

  // edges
  {
      { 0, 1 },
      { 0, 2 },
      { 0, 3 },
      { 0, 4 },
      { 1, 2 },
      { 2, 3 },
      { 3, 4 },
      { 4, 1 },
  },

  // start positions
  {
      { 1, 3 },
      { 2, 4 },
  },
};

const Board simple_diamond = {
  // nodes
  {
      { -100, 0 },
      { 0, 100 },
      { 100, 0 },
      { 0, -100 },
  },

  // edges
  {
      { 0, 1 },
      { 1, 2 },
      { 2, 3 },
      { 3, 0 },
  },

  // start positions
  {
      { 0, 2 },
      { 1, 3 },
  },
};

const Board hexagon = {
  // nodes
  {
      { 0, 0 },
      { -100, 0 },
      { -50, 100 },
      { 50, 100 },
      { 100, 0 },
      { 50, -100 },
      { -50, -100 },
  },

  // edges
  {
      { 0, 1 },
      { 0, 2 },
      { 0, 3 },
      { 0, 4 },
      { 0, 5 },
      { 0, 6 },
      { 1, 2 },
      { 2, 3 },
      { 3, 4 },
      { 4, 5 },
      { 5, 6 },
      { 6, 1 },
  },

  // start positions
  {
      { 1, 4 },
      { 2, 5 },
      { 3, 6 },
  },
};

const Board simple_hexagon = {
  // nodes
  {
      { 0, 0 },
      { -100, 0 },
      { -50, 100 },
      { 50, 100 },
      { 100, 0 },
      { 50, -100 },
      { -50, -100 },
  },

  // edges
  {
      { 0, 1 },
      { 0, 4 },
      { 1, 2 },
      { 2, 3 },
      { 3, 4 },
      { 4, 5 },
      { 5, 6 },
      { 6, 1 },
  },

  // start positions
  {
      { 1, 4 },
  },
};

Board::Board(initializer_list<Node> nodes,
             initializer_list<Edge> edges,
             initializer_list<StartNodes> start_nodes)
    : nodes(nodes), start_nodes(start_nodes) {
  // build the arcs list from edges
  for (const auto& edge : edges) {
    assert(edge.src != edge.dst);
    const auto& src_node = this->nodes[edge.src];
    const auto& dst_node = this->nodes[edge.dst];
    double dx = src_node.x - dst_node.x;
    double dy = src_node.y - dst_node.y;
    double dist = sqrt(dx * dx + dy * dy);
    arcs.push_back({ edge.src, edge.dst, dist });
    arcs.push_back({ edge.dst, edge.src, dist });
  }
}

const Board* Board::getBoard(BoardConfiguration configuration) {
  switch (configuration) {
    case BoardConfiguration::Triangle:
      return &triangle;
    case BoardConfiguration::Diamond:
      return &diamond;
    case BoardConfiguration::SimplifiedDiamond:
      return &simple_diamond;
    case BoardConfiguration::Hexagon:
      return &hexagon;
    case BoardConfiguration::SimplifiedHexagon:
      return &simple_hexagon;
  }

  FATAL("Unknown board configuration");
}

}  // namespace conquest
