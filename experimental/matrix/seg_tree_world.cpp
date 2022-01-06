
#include "seg_tree_world.h"

#include <core/math_2d.h>

#include <utility>
#include <random>
#include <cmath>
#include <limits>
#include <array>
using namespace std;

SegTreeWorld::SegTreeWorld() : World(sim::Rect(-kWidth / 2, -kHeight / 2, kWidth, kHeight)) {
  // walls
  b2BodyDef walls_def;
  auto walls = world_.CreateBody(&walls_def);

  b2EdgeShape wall_shape;
  b2FixtureDef wall_fixture_def;
  wall_fixture_def.shape = &wall_shape;
  wall_fixture_def.friction = 10.0f;
  wall_fixture_def.restitution = 0.8f;
  wall_fixture_def.material.color = b2Color(0.2, 0.5, 0.1);
  wall_fixture_def.material.emit_intensity = 0.1f;

  const b2Vec2 top_left(-kWidth / 2, kHeight / 2);
  const b2Vec2 top_right(kWidth / 2, kHeight / 2);
  const b2Vec2 bottom_left(-kWidth / 2, -kHeight / 2);
  const b2Vec2 bottom_right(kWidth / 2, -kHeight / 2);

  wall_shape.Set(bottom_left, bottom_right);
  walls->CreateFixture(&wall_fixture_def);
  wall_shape.Set(bottom_left, top_left);
  walls->CreateFixture(&wall_fixture_def);
  wall_shape.Set(bottom_right, top_right);
  walls->CreateFixture(&wall_fixture_def);
  wall_shape.Set(top_left, top_right);
  walls->CreateFixture(&wall_fixture_def);

  std::random_device rd;
  std::default_random_engine rnd(rd());
  uniform_real_distribution<float> dist_x(-kWidth / 2, kWidth / 2);
  uniform_real_distribution<float> dist_y(-kHeight / 2, kHeight / 2);
  uniform_real_distribution<float> dist_v(-10, 10);
  uniform_real_distribution<float> dist_wall_size(0.5, 5);
  uniform_real_distribution<float> dist_angle(0, 2 * math::kPi);

  for (int i = 0; i < 150; ++i) {
    const float x = dist_x(rnd);
    const float y = dist_y(rnd);
    const float w = dist_wall_size(rnd);
    const float h = dist_wall_size(rnd);

    b2PolygonShape shape;
    shape.SetAsBox(w, h, b2Vec2(x, y), dist_angle(rnd));

    wall_fixture_def.shape = &shape;
    walls->CreateFixture(&wall_fixture_def);
  }

  for (int i = 0; i < 5000; ++i) {
  retry:
    const b2Vec2 pos(dist_x(rnd), dist_y(rnd));

    for (auto fixture = walls->GetFixtureList(); fixture; fixture = fixture->GetNext()) {
      if (fixture->TestPoint(pos)) {
        goto retry;
      }
    }

    const auto body = addRobot(pos);
    body->SetLinearVelocity(b2Vec2(dist_v(rnd), dist_v(rnd)));
  }
}

    b2Body* SegTreeWorld::addRobot(const b2Vec2& pos) {
  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  body_def.position = pos;
  auto body = world_.CreateBody(&body_def);

      array<b2Vec2, 3> points = {
        b2Vec2(0, 0.3),
        b2Vec2(-0.1, 0),
        b2Vec2(0.1, 0),
        };

  b2CircleShape shape;
  shape.m_radius = 0.1f;

  b2FixtureDef fixture_def;
  fixture_def.shape = &shape;
  fixture_def.density = 1.0f;
  fixture_def.friction = 0.3f;
  fixture_def.restitution = 0.8f;
  fixture_def.material.color = b2Color(1.0, 0.5, 0.1);
  fixture_def.material.emit_intensity = 0.1f;
  body->CreateFixture(&fixture_def);

  b2PolygonShape shape2;
  shape2.Set(points.data(), points.size());

  b2FixtureDef fixture_def2;
  fixture_def2.shape = &shape2;
  fixture_def2.density = 1.0f;
  fixture_def2.friction = 0.3f;
  fixture_def2.restitution = 0.8f;
  fixture_def2.material.color = b2Color(1.0, 0.5, 0.3);
  fixture_def2.material.emit_intensity = 0.1f;
  body->CreateFixture(&fixture_def2);

  return body;
}
