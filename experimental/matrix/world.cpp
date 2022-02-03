
#include "world.h"

#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QPainter>
#include <QPolygonF>
#include <QRectF>

#include <utility>
#include <cmath>
#include <limits>
#include <thread>
using namespace std;

World::World(const sim::Rect& extents)
    : world_(b2Vec2(0, 0)),
      extents_(extents),
      contact_listener_(this),
      visual_map_(extents.width * kVisualMapScale,
                  extents.height * kVisualMapScale,
                  QImage::Format::Format_RGB32) {
  setupVisualMapTransformation();
  world_.SetContactListener(&contact_listener_);
  sim_state_ = SimState::Paused;
  new std::thread(&World::simThread, this);
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

void World::extractVisibleState() {
  // reset the staging data
  // (expecting that clear() would not free the memory)
  snapshot_staging_.clear();

  for (const b2Body* body = world_.GetBodyList(); body; body = body->GetNext()) {
    vis::Object object;
    object.center = body->GetWorldCenter();

    float radius_squared = 0;
    auto update_radius = [&radius_squared, body](const b2Vec2& pos) {
      const auto p = pos - body->GetLocalCenter();
      radius_squared = max(radius_squared, p.x * p.x + p.y * p.y);
    };

    for (const b2Fixture* fixture = body->GetFixtureList(); fixture;
         fixture = fixture->GetNext()) {
      switch (fixture->GetType()) {
        case b2Shape::e_circle: {
          auto shape = static_cast<const b2CircleShape*>(fixture->GetShape());
          vis::Circle circle;
          circle.color = fixture->GetMaterial().color;
          circle.center = body->GetWorldPoint(shape->m_p);
          circle.radius = shape->m_radius;
          object.base_color = circle.color;
          object.circles.push_back(circle);
          const auto p = circle.center - body->GetWorldCenter();
          const float dist = p.Length() + circle.radius;
          radius_squared = max(radius_squared, dist * dist);
          break;
        }
        case b2Shape::e_edge: {
          auto shape = static_cast<const b2EdgeShape*>(fixture->GetShape());
          vis::Edge edge;
          edge.color = fixture->GetMaterial().color;
          edge.a = body->GetWorldPoint(shape->m_vertex1);
          edge.b = body->GetWorldPoint(shape->m_vertex2);
          update_radius(shape->m_vertex1);
          update_radius(shape->m_vertex2);
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
            polygon.points.push_back(body->GetWorldPoint(p));
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

#if 0
    int contacts_count = 0;
    for (const auto* ce = body->GetContactList(); ce != nullptr; ce = ce->next) {
      const int point_count = ce->contact->GetManifold()->pointCount;
      if (ce->contact->IsTouching()) {
        CHECK(point_count > 0);
        b2WorldManifold manifold = {};
        ce->contact->GetWorldManifold(&manifold);
        vis::Circle circle;
        circle.color = b2Color(1, 0, 0);
        circle.radius = 0.03f;
        for (int i = 0; i < point_count; ++i) {
          circle.center = manifold.points[i];
          object.circles.push_back(circle);
        }
        ++contacts_count;
      }
    }
    if (contacts_count > max_contacts) {
      max_contacts = contacts_count;
    }
    if (contacts_count > 25 && body->GetType() != b2_staticBody) {
      const b2Color highlight_color(0, 0, 1);
      object.base_color = highlight_color;
      for (auto& s : object.edges) {
        s.color = highlight_color;
      }
      for (auto& s : object.circles) {
        s.color = highlight_color;
      }
      for (auto& s : object.polygons) {
        s.color = highlight_color;
      }
    }
#endif

    snapshot_staging_.push_back(std::move(object));
  }
}

void World::setupVisualMapTransformation() {
  visual_map_transformation_.reset();
  visual_map_transformation_.scale(kVisualMapScale, -kVisualMapScale);
  visual_map_transformation_.translate(-extents_.x, -(extents_.y + extents_.height));
}

static QPointF vecToPoint(const b2Vec2& v) {
  return QPointF(v.x, v.y);
}

void World::renderVisualMap() {
  QPainter painter(&visual_map_);

  painter.setRenderHints(QPainter::Antialiasing);
  painter.setTransform(visual_map_transformation_);

  const QRectF map_rect(extents_.x, extents_.y, extents_.width, extents_.height);
  painter.setBrush(QBrush(Qt::white));
  painter.setPen(Qt::NoPen);
  painter.drawRect(map_rect);

  for (const auto& obj : snapshot_) {
    // edges
    for (const auto& edge : obj.edges) {
      const auto& color = edge.color;
      painter.setPen(QPen(QColor::fromRgbF(color.r, color.g, color.b), 0));
      const auto a = vecToPoint(edge.a);
      const auto b = vecToPoint(edge.b);
      painter.drawLine(a, b);
    }

    // circles
    for (const auto& circle : obj.circles) {
      const auto& color = circle.color;
      painter.setPen(QPen(QColor::fromRgbF(color.r, color.g, color.b), 0));
      painter.setBrush(QColor::fromRgbF(color.r, color.g, color.b, 0.4));
      const auto p = vecToPoint(circle.center);
      painter.drawEllipse(p, circle.radius, circle.radius);
    }

    // polygons
    for (const auto& polygon : obj.polygons) {
      const auto& color = polygon.color;
      painter.setPen(QPen(QColor::fromRgbF(color.r, color.g, color.b), 0));
      painter.setBrush(QColor::fromRgbF(color.r, color.g, color.b, 0.4));
      QPolygonF shape;
      for (const auto& p : polygon.points) {
        shape.append(vecToPoint(p));
      }
      painter.drawPolygon(shape);
    }
  }
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

    extractVisibleState();

    {
      unique_lock<mutex> guard(snapshot_lock_);
      swap(snapshot_, snapshot_staging_);
      ups_ = ups_tracker_.currentRate();
    }

#if 0
    renderVisualMap();
#endif
  }
}

void World::simStep() {
  constexpr float32 kTimeStep = 1.0f / 50.0f;
  constexpr int32 kVelocityIterations = 10;
  constexpr int32 kPositionIterations = 10;
  preStep();
  world_.Step(kTimeStep, kVelocityIterations, kPositionIterations);
  timestamp_ += kTimeStep;
  postStep(kTimeStep);
}
