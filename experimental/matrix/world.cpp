
#include "world.h"

#include <utility>
#include <cmath>
#include <limits>
#include <thread>
using namespace std;

World::World(const sim::Rect& extents) : world_(b2Vec2(0, 0)), extents_(extents) {
  new std::thread(&World::simThread, this);
  sim_state_ = SimState::Paused;
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
