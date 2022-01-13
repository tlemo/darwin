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

#include <core/sim/track.h>
#include <core/random.h>
#include <third_party/box2d/box2d.h>
#include <third_party/gtest/gtest.h>

#include <random>
using namespace std;

namespace track_tests {

TEST(TrackTest, DefaultConfig) {
  // track configuration
  sim::TrackConfig track_config;

  constexpr int kIterations = 100;
  for (int i = 0; i < kIterations; ++i) {
    b2World world(b2Vec2(0, 0));
    sim::Track test_track(core::randomSeed(), track_config);
    test_track.createFixtures(&world);
  }
}

TEST(TrackTest, HighComplexity) {
  // track configuration
  sim::TrackConfig track_config;
  track_config.width = 2.5f;
  track_config.complexity = 35;
  track_config.resolution = 333;
  track_config.area_width = 20.0f;
  track_config.area_height = 50.0f;
  track_config.curb_width = 0.5f;
  track_config.gates = true;

  constexpr int kIterations = 100;
  for (int i = 0; i < kIterations; ++i) {
    b2World world(b2Vec2(0, 0));
    sim::Track test_track(core::randomSeed(), track_config);
    test_track.createFixtures(&world);
  }
}

TEST(TrackTest, HighResolution) {
  // track configuration
  sim::TrackConfig track_config;
  track_config.width = 5.0f;
  track_config.complexity = 25;
  track_config.resolution = 1500;
  track_config.area_width = 20.0f;
  track_config.area_height = 100.0f;
  track_config.curb_width = 0.01f;
  track_config.gates = true;

  constexpr int kIterations = 25;
  for (int i = 0; i < kIterations; ++i) {
    b2World world(b2Vec2(0, 0));
    sim::Track test_track(core::randomSeed(), track_config);
    test_track.createFixtures(&world);
  }
}

TEST(TrackTest, LowResolution) {
  // track configuration
  sim::TrackConfig track_config;
  track_config.width = 1.0f;
  track_config.complexity = 15;
  track_config.resolution = 9;
  track_config.area_width = 10.0f;
  track_config.area_height = 100.0f;
  track_config.curb_width = 0.3f;
  track_config.gates = true;

  constexpr int kIterations = 100;
  for (int i = 0; i < kIterations; ++i) {
    b2World world(b2Vec2(0, 0));
    sim::Track test_track(core::randomSeed(), track_config);
    test_track.createFixtures(&world);
  }
}

}  // namespace track_tests
