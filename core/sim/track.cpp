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

#include "track.h"

#include <core/math_2d.h>
#include <third_party/tinyspline/tinyspline.h>

namespace sim {

Track::Track(Track::Seed seed, b2World* world, const TrackConfig& config)
    : rnd_(seed), config_(config) {
  generateTrackPath();
  createFixtures(world);
}

int Track::updateTrackDistance(int old_distance, const b2Vec2& pos) const {
  auto side = [&](int distance) {
    const auto& node = track_nodes_[distanceToNode(distance)];
    return b2Cross(node.normal, pos - node.pos);
  };

  int new_distance = old_distance;

  // move forward?
  while (side(new_distance + 1) >= 0) {
    ++new_distance;
  }

  // move backward?
  if (new_distance == old_distance) {
    while (side(new_distance) < 0) {
      --new_distance;
    }
  }

  return new_distance;
}

int Track::distanceToNode(int distance) const {
  const int n = int(track_nodes_.size());
  int index = distance;
  while (index < 0) {
    index += n;
  }
  return index % n;
}

void Track::generateTrackPath() {
  CHECK(track_nodes_.empty());

  // generate random control points (counter-clockwise, around the center)
  const float radius = (config_.area_width + config_.area_height) / 2.0f;
  std::uniform_real_distribution<double> dist(radius * 0.1f, radius);
  const double kLimitX = config_.area_width / 2 - config_.width;
  const double kLimitY = config_.area_height / 2 - config_.width;
  vector<math::Vector2d> control_points(config_.complexity);
  for (size_t i = 0; i < control_points.size(); ++i) {
    const double angle = i * math::kPi * 2 / config_.complexity;
    const double r = dist(rnd_);
    control_points[i].x = max(min(cos(angle) * r, kLimitX), -kLimitX);
    control_points[i].y = max(min(sin(angle) * r, kLimitY), -kLimitY);
  }

  // create the track spline (as a closed curve)
  const size_t n = control_points.size() + 3;
  tinyspline::BSpline spline(n, 2, 3, TS_OPENED);
  auto cp = spline.controlPoints();
  for (size_t i = 0; i < n; ++i) {
    cp[i * 2 + 0] = control_points[i % control_points.size()].x;
    cp[i * 2 + 1] = control_points[i % control_points.size()].y;
  }
  spline.setControlPoints(cp);

  // sample evenly spaced points from the spline
  // (we defined a closed curve - first and last point overlap, so drop the last one)
  const size_t samples_count = config_.resolution;
  auto samples = spline.sample(samples_count + 1);
  samples.pop_back();

  // set the track points
  track_nodes_.resize(samples_count);
  for (size_t i = 0; i < samples_count; ++i) {
    track_nodes_[i].pos.x = float(samples[i * 2 + 0]);
    track_nodes_[i].pos.y = float(samples[i * 2 + 1]);
  }

  // calculate the normals
  for (size_t i = 0; i < samples_count; ++i) {
    const size_t next_i = (i + 1) % samples_count;
    const auto next_v = track_nodes_[next_i].pos - track_nodes_[i].pos;
    const auto next_n = b2Vec2(next_v.y, -next_v.x).Normalized();

    const size_t prev_i = (i + samples_count - 1) % samples_count;
    const auto prev_v = track_nodes_[i].pos - track_nodes_[prev_i].pos;
    const auto prev_n = b2Vec2(prev_v.y, -prev_v.x).Normalized();

    track_nodes_[i].normal = (prev_n + next_n) * 0.5f;
  }
}

void Track::createFixtures(b2World* world) {
  CHECK(!track_nodes_.empty());

  b2BodyDef track_body_def;
  auto track_body = world->CreateBody(&track_body_def);

  const b2Color red(1, 0, 0);
  const b2Color white(1, 1, 1);
  const b2Color blue(0, 0, 1);

  bool primary_color = true;
  for (size_t i = 0; i < track_nodes_.size(); ++i) {
    const size_t next_i = (i + 1) % track_nodes_.size();

    // common definitions
    b2Vec2 points[4];
    b2PolygonShape shape;

    b2FixtureDef fixture_def;
    fixture_def.shape = &shape;
    fixture_def.friction = 0.2f;
    fixture_def.restitution = 0.5f;
    fixture_def.material.emit_intensity = 0;

    // left side curb
    points[0] = track_nodes_[i].pos;
    points[1] = track_nodes_[next_i].pos;
    points[2] = track_nodes_[next_i].offsetPos(-config_.curb_width);
    points[3] = track_nodes_[i].offsetPos(-config_.curb_width);
    shape.Set(points, 4);

    fixture_def.material.color = primary_color ? red : white;
    track_body->CreateFixture(&fixture_def);

    // right side curb
    points[0] = track_nodes_[i].offsetPos(config_.width + config_.curb_width);
    points[1] = track_nodes_[next_i].offsetPos(config_.width + config_.curb_width);
    points[2] = track_nodes_[next_i].offsetPos(config_.width);
    points[3] = track_nodes_[i].offsetPos(config_.width);
    shape.Set(points, 4);

    fixture_def.material.color = primary_color ? blue : white;
    track_body->CreateFixture(&fixture_def);

    primary_color = !primary_color;
  }
}

}  // namespace sim
