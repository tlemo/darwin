
#include "world.h"

#include <core/math_2d.h>

#include <utility>
#include <random>
#include <cmath>
#include <limits>
#include <thread>
using namespace std;

World::World() : world_(b2Vec2(0, 0)) {
  new std::thread(&World::simThread, this);
}

void World::generateWorld() {
  printf("Generating world...\n");

  unique_lock<mutex> guard(state_lock_);
  CHECK(sim_state_ == SimState::Invalid);

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

  for (int i = 0; i < 250; ++i) {
    const float x = dist_x(rnd);
    const float y = dist_y(rnd);
    const float w = dist_wall_size(rnd);
    const float h = dist_wall_size(rnd);

    b2PolygonShape shape;
    shape.SetAsBox(w, h, b2Vec2(x, y), dist_angle(rnd));

    wall_fixture_def.shape = &shape;
    walls->CreateFixture(&wall_fixture_def);
  }

  for (int i = 0; i < 10000; ++i) {
  retry:
    const b2Vec2 pos(dist_x(rnd), dist_y(rnd));

    for (auto fixture = walls->GetFixtureList(); fixture; fixture = fixture->GetNext()) {
      if (fixture->TestPoint(pos)) {
        goto retry;
      }
    }

    const auto body = addBall(pos);
    body->SetLinearVelocity(b2Vec2(dist_v(rnd), dist_v(rnd)));
  }

  sim_state_ = SimState::Paused;
  state_cv_.notify_all();
}

void World::runSimulation() {
  unique_lock<mutex> guard(state_lock_);
  CHECK(sim_state_ != SimState::Invalid);
  sim_state_ = SimState::Running;
  state_cv_.notify_all();
}

void World::pauseSimulation() {
  unique_lock<mutex> guard(state_lock_);
  CHECK(sim_state_ != SimState::Invalid);
  sim_state_ = SimState::Paused;
  state_cv_.notify_all();
}

const vis::World World::visibleState() {
  unique_lock<mutex> guard(snapshot_lock_);
  return snapshot_;
}

vis::World World::extractVisibleState() const {
  vis::World vis_world;

  for (const b2Body* body = world_.GetBodyList(); body; body = body->GetNext()) {
    vis::Object object;
    object.xf = body->GetTransform();

    float radius_squared = 0;
    auto update_radius = [&radius_squared](const b2Vec2& p) {
      radius_squared = max(radius_squared, p.x * p.x + p.y * p.y);
    };

    for (const b2Fixture* fixture = body->GetFixtureList(); fixture;
         fixture = fixture->GetNext()) {
      switch (fixture->GetType()) {
        case b2Shape::e_circle: {
          auto shape = static_cast<const b2CircleShape*>(fixture->GetShape());
          vis::Circle circle;
          circle.color = fixture->GetMaterial().color;
          circle.center = shape->m_p;
          circle.radius = shape->m_radius;
          object.base_color = circle.color;
          object.circles.push_back(circle);
          const float dist = circle.center.Length() + circle.radius;
          radius_squared = max(radius_squared, dist * dist);
          break;
        }
        case b2Shape::e_edge: {
          auto shape = static_cast<const b2EdgeShape*>(fixture->GetShape());
          vis::Edge edge;
          edge.color = fixture->GetMaterial().color;
          edge.a = shape->m_vertex1;
          edge.b = shape->m_vertex2;
          update_radius(edge.a);
          update_radius(edge.b);
          object.base_color = edge.color;
          object.edges.push_back(edge);
          break;
        }
        case b2Shape::e_polygon: {
          auto shape = static_cast<const b2PolygonShape*>(fixture->GetShape());
          vis::Polygon polygon;
          polygon.color = fixture->GetMaterial().color;
          for (int i = 0; i < shape->m_count; ++i) {
            const auto& p = shape->m_vertices[i];
            polygon.points.push_back(p);
            update_radius(p);
          }
          object.base_color = polygon.color;
          object.polygons.push_back(std::move(polygon));
          break;
        }
        default:
          FATAL("Unexpected fixture shape");
      }
    }

    object.radius = sqrtf(radius_squared);
    vis_world.push_back(std::move(object));
  }

  return vis_world;
}

b2Body* World::addBall(const b2Vec2& pos) {
  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  body_def.position = pos;
  auto body = world_.CreateBody(&body_def);

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

  return body;
}

void World::simThread() {
  for (;;) {
    {
      unique_lock<mutex> guard(state_lock_);
      while (sim_state_ != SimState::Running) {
        state_cv_.wait(guard);
      }
    }

    simStep();
    ups_tracker_.update();

    auto visible_state = extractVisibleState();

    {
      unique_lock<mutex> guard(snapshot_lock_);
      swap(snapshot_, visible_state);
      ups_ = ups_tracker_.currentRate();
    }
  }
}

void World::simStep() {
  constexpr float32 kTimeStep = 1.0f / 50.0f;
  constexpr int32 kVelocityIterations = 10;
  constexpr int32 kPositionIterations = 10;
  world_.Step(kTimeStep, kVelocityIterations, kPositionIterations);
  timestamp_ += kTimeStep;
}
