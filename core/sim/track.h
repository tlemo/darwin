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

#include <core/utils.h>
#include <third_party/box2d/box2d.h>

#include <vector>
#include <random>
using namespace std;

namespace sim {

struct TrackConfig {
  float width = 1.8f;         //!< Track width (curb to curb)
  int complexity = 10;        //!< The approximate number of turns
  int resolution = 500;       //!< Number of track segments
  float area_width = 40.0f;   //!< The width of the track area
  float area_height = 20.0f;  //!< The height of the track area
  float curb_width = 0.1f;    //!< Curb width
};

struct TrackNode {
  b2Vec2 pos;
  b2Vec2 normal;

  b2Vec2 offsetPos(float offset) const { return pos + normal * offset; }
};

class Track : public core::NonCopyable {
 public:
  using Seed = std::random_device::result_type;

 public:
  Track(Seed seed, b2World* world, const TrackConfig& config);

  const vector<TrackNode>& trackNodes() const { return track_nodes_; }

  int updateTrackDistance(int old_distance, const b2Vec2& pos) const;
  int distanceToNode(int distance) const;

 private:
  void generateTrackPath();
  void createFixtures(b2World* world);

 private:
  default_random_engine rnd_;
  vector<TrackNode> track_nodes_;
  const TrackConfig& config_;
};

}  // namespace sim
