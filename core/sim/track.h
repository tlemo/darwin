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
#include <core/math_2d.h>
#include <core/outline_2d.h>
#include <third_party/box2d/box2d.h>

#include <vector>
#include <random>
using namespace std;

namespace sim {

struct TrackConfig {
  float width = 1.8f;            //!< Track width (curb to curb)
  int complexity = 10;           //!< The approximate number of turns
  int resolution = 500;          //!< Number of track segments
  float area_width = 40.0f;      //!< The width of the track area
  float area_height = 20.0f;     //!< The height of the track area
  float curb_width = 0.1f;       //!< Curb width
  bool gates = true;             //!< Generate gate fixtures
  bool solid_gate_posts = true;  //!< Can objects collide with the gate posts?
  float curb_friction = 0.5f;    //!< Curb friction
};

class Track : public core::NonCopyable {
 public:
  using Seed = std::random_device::result_type;

 public:
  Track(Seed seed, const TrackConfig& config);

  int nodesCount() const { return int(outer_outline_.nodes().size()); }
  int updateTrackDistance(int old_distance, const b2Vec2& pos) const;
  const math::Outline::Node& distanceToNode(int distance) const;

  void createFixtures(b2World* world) const;

  // mostly intended for rendering tracks, everything else
  // should use the distance methods
  const math::Outline& innerOutline() const { return inner_outline_; }
  const math::Outline& outerOutline() const { return outer_outline_; }

 private:
  int distanceToNodeIndex(int distance) const;
  void generateTrackPath();

  void createCurb(b2World* world,
                  const b2Color& color,
                  const math::Outline& outline,
                  float curb_width) const;

  void createGates(b2World* world) const;

 private:
  default_random_engine rnd_;
  const TrackConfig& config_;

  math::Outline inner_outline_;
  math::Outline outer_outline_;
};

}  // namespace sim
