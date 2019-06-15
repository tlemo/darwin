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

#include <domains/ballistics/world.h>

#include <core/math_2d.h>
#include <third_party/gtest/gtest.h>

#include <memory>
#include <vector>
using namespace std;

namespace ballistics_tests {

TEST(BallisticsTest, World_Projectile) {
  ballistics::Config config;
  config.range_min_x = -10;
  config.range_max_x = 10;
  config.range_min_y = -10;
  config.range_max_y = 10;
  config.target_radius = 1.0f;
  config.target_hit_bonus = 0.0f;
  config.projectile_radius = 2.5f;
  config.projectile_velocity = 30.0f;
  ballistics::Ballistics domain(config);

  constexpr int kMaxSteps = 500;

  struct SimulationSummary {
    float closest_distance;
    b2Vec2 peak_position;
    b2Vec2 final_position;
  };

  auto simulation = [&](float aim_angle) -> SimulationSummary {
    SimulationSummary summary = {};

    const auto target_position = b2Vec2(0, -5);
    const float target_distance = target_position.Length();

    ballistics::World world(target_position, &domain);
    world.fireProjectile(aim_angle);

    summary.closest_distance = target_distance;
    summary.peak_position = b2Vec2(0, 0);

    int step = 0;
    do {
      EXPECT_LE(step, kMaxSteps);
      const auto projectile_position = world.projectilePosition();
      const float distance = (projectile_position - target_position).Length();
      if (distance < summary.closest_distance) {
        summary.closest_distance = distance;
      }
      if (projectile_position.y >= summary.peak_position.y) {
        summary.peak_position = projectile_position;
      }
      ++step;
    } while (world.simStep());
    EXPECT_GT(step, 0);

    summary.final_position = world.projectilePosition();
    return summary;
  };

  auto shoot_right_summary = simulation(0.0f);
  EXPECT_EQ(shoot_right_summary.closest_distance, 5.0f);
  EXPECT_EQ(shoot_right_summary.peak_position.x, 0.0f);
  EXPECT_EQ(shoot_right_summary.peak_position.y, 0.0f);
  EXPECT_GT(shoot_right_summary.final_position.x, 0.0f);
  EXPECT_LE(shoot_right_summary.final_position.y, -5.0f);

  auto shoot_left_summary = simulation(float(math::kPi));
  EXPECT_EQ(shoot_left_summary.closest_distance, 5.0f);
  EXPECT_EQ(shoot_left_summary.peak_position.x, 0.0f);
  EXPECT_EQ(shoot_left_summary.peak_position.y, 0.0f);
  EXPECT_LT(shoot_left_summary.final_position.x, 0.0f);
  EXPECT_LE(shoot_left_summary.final_position.y, -5.0f);

  auto shoot_up_summary = simulation(float(math::kPi / 2));
  EXPECT_LT(shoot_up_summary.closest_distance, 0.5f);
  EXPECT_GT(shoot_up_summary.peak_position.y, 0.0f);
  EXPECT_LE(shoot_up_summary.final_position.y, -5.0f);

  auto shoot_down_summary = simulation(float(-math::kPi / 2));
  EXPECT_LT(shoot_down_summary.closest_distance, 0.5f);
  EXPECT_EQ(shoot_down_summary.peak_position.y, 0.0f);
  EXPECT_LE(shoot_down_summary.final_position.y, -5.0f);
}

}  // namespace ballistics_tests
