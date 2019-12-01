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

#include "domain.h"

#include <third_party/box2d/box2d.h>

#include <vector>
#include <random>
using namespace std;

namespace drone_track {

struct TrackNode {
  b2Vec2 pos;
  b2Vec2 normal;

  b2Vec2 offsetPos(float offset) const { return pos + normal * offset; }
};

class Track : public core::NonCopyable {
  static constexpr float kWidth = 40;
  static constexpr float kHeight = 20;
  static constexpr float kCurbWidth = 0.1f;

 public:
  using Seed = std::random_device::result_type;

 public:
  Track(Seed seed, b2World* world, const Config& config);

  const vector<TrackNode>& trackNodes() const { return track_nodes_; }

  int updateTrackDistance(int old_distance, const b2Vec2& pos) const;
  int distanceToNode(int distance) const;

 private:
  void generateTrackPath();
  void createFixtures(b2World* world);

 private:
  default_random_engine rnd_;
  vector<TrackNode> track_nodes_;
  const Config& config_;
};

}  // namespace drone_track
