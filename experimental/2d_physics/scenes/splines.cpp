
#include "splines.h"

#include <core/global_initializer.h>
#include <core/math_2d.h>
#include <third_party/tinyspline/tinyspline.h>

#include <QPainter>
#include <QBrush>
#include <QPen>
#include <QPointF>
#include <QRectF>
#include <QLineF>
#include <QPainterPath>

#include <random>
using namespace std;

namespace splines_scene {

GLOBAL_INITIALIZER {
  scenesRegistry().add<Factory>("Splines");
}

Scene::Scene(const core::PropertySet* config) : sim::Scene(b2Vec2(0, 0), sim::Rect()) {
  if (config) {
    config_.copyFrom(*config);
  }

  const auto width = config_.width;
  const auto height = config_.height;
  setExtents(sim::Rect(-width / 2, -height / 2, width, height));

  updateVariables();
}

void Scene::preStep() {
  // placeholder
}

void Scene::postStep(float /*dt*/) {
  updateVariables();
}

void Scene::addBalloon(float x, float y, float radius) {
  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  body_def.position.Set(x, y);
  body_def.linearDamping = 1.0f;
  body_def.angularDamping = 1.0f;
  auto body = world_.CreateBody(&body_def);

  b2CircleShape shape;
  shape.m_radius = radius;

  b2FixtureDef fixture_def;
  fixture_def.shape = &shape;
  fixture_def.density = 0.02f;
  fixture_def.friction = 1.0f;
  fixture_def.restitution = 0.9f;
  fixture_def.material.color = b2Color(1, 0, 0);
  fixture_def.material.shininess = 10;
  fixture_def.material.emit_intensity = 0.1f;
  body->CreateFixture(&fixture_def);
}

void Scene::addBox(float x, float y, float sx, float sy) {
  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  body_def.position.Set(x, y);
  body_def.linearDamping = 2.0f;
  body_def.angularDamping = 2.0f;
  auto body = world_.CreateBody(&body_def);

  b2PolygonShape shape;
  shape.SetAsBox(sx, sy);

  b2FixtureDef fixture_def;
  fixture_def.shape = &shape;
  fixture_def.density = 0.5f;
  fixture_def.friction = 1.0f;
  fixture_def.restitution = 0.5f;
  fixture_def.material.color = b2Color(0, 1, 0);
  fixture_def.material.shininess = 25;
  fixture_def.material.emit_intensity = 0.1f;
  body->CreateFixture(&fixture_def);
}

void Scene::clear() {
  auto body = world_.GetBodyList();
  while (body != nullptr) {
    auto next = body->GetNext();
    world_.DestroyBody(body);
    body = next;
  }
}

void Scene::createLight(b2Body* body, const b2Vec2& pos, const b2Color& color) {
  b2LightDef light_def;
  light_def.body = body;
  light_def.color = color;
  light_def.intensity = 2.0f;
  light_def.attenuation_distance = 25.0f;
  light_def.position = pos;
  world_.CreateLight(&light_def);
}

void Scene::updateVariables() {
  variables_.objects_count = world_.GetBodyCount();
}

void SceneUi::renderSpline(QPainter& painter,
                           const vector<math::Vector2d> control_points) const {
  // render control points
  painter.setBrush(Qt::NoBrush);
  QPainterPath cp_path;
  const auto& last_cp = control_points.back();
  cp_path.moveTo(last_cp.x, last_cp.y);
  for (const auto& cp : control_points) {
    painter.setPen(QPen(Qt::green, 0));
    painter.drawEllipse(QPointF(cp.x, cp.y), 0.1, 0.1);
    cp_path.lineTo(cp.x, cp.y);
    painter.setPen(QPen(Qt::lightGray, 0, Qt::DotLine));
    painter.drawLine(QLineF(0, 0, cp.x, cp.y));
  }
  painter.setPen(QPen(Qt::green, 0, Qt::DotLine));
  painter.drawPath(cp_path);

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
  const size_t samples_count = scene_->config()->track_resolution;
  auto samples = spline.sample(samples_count + 1);
  samples.pop_back();

  // render the spline
  QPainterPath spline_path;
  for (size_t i = 0; i < samples_count; ++i) {
    const auto x = samples[i * 2 + 0];
    const auto y = samples[i * 2 + 1];
    if (i == 0) {
      const size_t last = samples_count - 1;
      const auto last_x = samples[last * 2 + 0];
      const auto last_y = samples[last * 2 + 1];
      spline_path.moveTo(last_x, last_y);
    }
    spline_path.lineTo(x, y);
  }
  painter.setPen(QPen(Qt::blue, 0, Qt::DashLine));
  painter.setBrush(Qt::NoBrush);
  painter.drawPath(spline_path);
}

void SceneUi::render(QPainter& painter, const QRectF&) {
  // inner spline
  renderSpline(painter, control_points_);

  // create the outer spline
  auto tmp_control_points = control_points_;
  const auto track_width = scene_->config()->track_width;
  for (auto& cp : tmp_control_points) {
    const auto len = cp.length();
    cp = cp * ((len + track_width) / len);
  }
  renderSpline(painter, tmp_control_points);
}

void SceneUi::step() {
  // placeholder
}

void SceneUi::mousePressEvent(const QPointF& pos, QMouseEvent* event) {
  const auto x = float(pos.x());
  const auto y = float(pos.y());

  if ((event->buttons() & Qt::LeftButton) != 0) {
    scene_->addBalloon(x, y, 0.8f);
  }

  if ((event->buttons() & Qt::RightButton) != 0) {
    scene_->addBox(x, y, 0.5f, 2.0f);
  }
}

void SceneUi::keyPressEvent(QKeyEvent* event) {
  key_state_[event->key()] = true;

  switch (event->key()) {
    case Qt::Key_N:
      generateRandomTrack();
      break;
    case Qt::Key_D:
      scene_->clear();
      break;
  }
}

void SceneUi::generateRandomTrack() {
  const auto config = scene_->config();
  const double x_limit = config->width / 2 - config->track_width;
  const double y_limit = config->height / 2 - config->track_width;
  const double radius = (config->width + config->height) / 2.0;
  std::uniform_real_distribution<double> dist(radius * 0.0f, radius);
  std::random_device rd;
  std::default_random_engine rnd(rd());
  control_points_.resize(config->track_complexity);
  for (size_t i = 0; i < config->track_complexity; ++i) {
    const double angle = i * math::kPi * 2 / config->track_complexity;
    const double r = dist(rnd);
    const double x = cos(angle) * r;
    const double y = sin(angle) * r;
    const double vt = (y >= 0 ? y_limit : -y_limit) / y;
    const double ht = (x >= 0 ? x_limit : -x_limit) / x;
    const double t = min(min(vt, ht), 1.0);
    control_points_[i].x = x * t;
    control_points_[i].y = y * t;
  }
}

}  // namespace splines_scene
