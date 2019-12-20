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

namespace sim {

Track::Track(Track::Seed seed, const TrackConfig& config) : rnd_(seed), config_(config) {
  CHECK(config_.complexity >= 3);
  generateTrackPath();
}

int Track::updateTrackDistance(int old_distance, const b2Vec2& pos) const {
  auto side = [&](int distance) {
    const auto& node = distanceToNode(distance);
    const math::Vector2d p(pos.x, pos.y);
    return node.n.cross(p - node.p);
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

const math::Outline::Node& Track::distanceToNode(int distance) const {
  const auto& nodes = outer_outline_.nodes();
  return nodes[distanceToNodeIndex(distance)];
}

int Track::distanceToNodeIndex(int distance) const {
  const int n = nodesCount();
  int index = distance;
  while (index < 0) {
    index += n;
  }
  return index % n;
}

void Track::generateTrackPath() {
  CHECK(inner_outline_.empty());
  CHECK(outer_outline_.empty());

  // generate random control points (counter-clockwise, around the center)
  const double x_limit = config_.area_width / 2 - config_.width;
  const double y_limit = config_.area_height / 2 - config_.width;
  const float radius = (config_.area_width + config_.area_height) / 2.0f;
  std::uniform_real_distribution<double> dist(0.1f, radius);
  math::Polygon control_points(config_.complexity);
  for (size_t i = 0; i < control_points.size(); ++i) {
    const double angle = i * math::kPi * 2 / config_.complexity;
    const double r = dist(rnd_);
    const double x = cos(angle) * r;
    const double y = sin(angle) * r;

    // truncate the point to the track area rectangle
    const double vt = (y >= 0 ? y_limit : -y_limit) / y;
    const double ht = (x >= 0 ? x_limit : -x_limit) / x;
    const double t = min(min(vt, ht), 1.0);
    control_points[i].x = x * t;
    control_points[i].y = y * t;
  }

  const int resolution = config_.resolution;

  // create the inner spline
  inner_outline_ = math::Outline(control_points, resolution).makeEquidistant();

  // create the outer spline
  auto outer_control_points = inner_outline_.offset(config_.width).toPolygon();
  outer_outline_ = math::Outline(outer_control_points, resolution).makeEquidistant();
}

static b2Vec2 toBox2dVec(const math::Vector2d& v) {
  return b2Vec2(float(v.x), float(v.y));
}

void Track::createCurb(b2World* world,
                       const b2Color& color,
                       const math::Outline& outline,
                       float curb_width) const {
  auto& nodes = outline.nodes();
  CHECK(nodes.size() >= 3);

  b2BodyDef body_def;
  auto curb_body = world->CreateBody(&body_def);

  const b2Color white(1, 1, 1);

  bool primary_color = true;
  for (size_t i = 0; i < nodes.size(); ++i) {
    const size_t next_i = (i + 1) % nodes.size();

    b2Vec2 points[4];
    b2PolygonShape shape;
    points[0] = toBox2dVec(nodes[i].p);
    points[1] = toBox2dVec(nodes[next_i].p);
    points[2] = toBox2dVec(nodes[next_i].offset(curb_width));
    points[3] = toBox2dVec(nodes[i].offset(curb_width));
    shape.Set(points, 4);

    b2FixtureDef fixture_def;
    fixture_def.shape = &shape;
    fixture_def.friction = config_.curb_friction;
    fixture_def.restitution = 0.5f;
    fixture_def.material.emit_intensity = 0;
    fixture_def.material.color = primary_color ? color : white;
    primary_color = !primary_color;

    curb_body->CreateFixture(&fixture_def);
  }
}

void Track::createGates(b2World* world) const {
  b2BodyDef body_def;
  auto gates_body = world->CreateBody(&body_def);

  b2CircleShape shape;
  shape.m_radius = config_.curb_width * 1.5f;

  b2FixtureDef fixture_def;
  fixture_def.shape = &shape;
  fixture_def.friction = config_.curb_friction;
  fixture_def.restitution = 0.5f;
  fixture_def.material.emit_intensity = 0.8f;
  fixture_def.material.color = b2Color(0, 1, 0);

  if (!config_.solid_gate_posts) {
    fixture_def.filter.maskBits = 0;
  }

  const auto& nodes = outer_outline_.nodes();
  const size_t gate_gap = nodes.size() / config_.complexity;
  const double mid_offset = config_.curb_width / 2;
  CHECK(gate_gap > 0);
  for (size_t i = 0; i < nodes.size(); i += gate_gap) {
    fixture_def.material.color = (i == 0 ? b2Color(0, 1, 0) : b2Color(0.7f, 0.7f, 0));

    // right (outer curb)
    const auto& outer_node = nodes[i];
    shape.m_p = toBox2dVec(outer_node.offset(mid_offset));
    gates_body->CreateFixture(&fixture_def);

    // left (inner curb)
    const auto& inner_node =
        inner_outline_.findClosestNode(outer_node.offset(-config_.width));
    shape.m_p = toBox2dVec(inner_node.offset(-mid_offset));
    gates_body->CreateFixture(&fixture_def);
  }
}

void Track::createFixtures(b2World* world) const {
  createCurb(world, b2Color(1, 0, 0), inner_outline_, -config_.curb_width);
  createCurb(world, b2Color(0, 0, 1), outer_outline_, config_.curb_width);
  if (config_.gates) {
    createGates(world);
  }
}

}  // namespace sim
