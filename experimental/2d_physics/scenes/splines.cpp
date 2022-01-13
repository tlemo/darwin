
#include "splines.h"

#include <core/global_initializer.h>
#include <core/random.h>

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

static b2Vec2 toBox2dVec(const math::Vector2d& v) {
  return b2Vec2(float(v.x), float(v.y));
}

void Scene::createCurb(const math::Outline& outline, float curb_width) {
  auto& nodes = outline.nodes();
  CHECK(nodes.size() >= 3);

  b2BodyDef track_body_def;
  auto track_body = world_.CreateBody(&track_body_def);

  const b2Color red(1, 0, 0);
  const b2Color blue(0, 0, 1);

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
    fixture_def.friction = 0.2f;
    fixture_def.restitution = 0.5f;
    fixture_def.material.emit_intensity = 0;
    fixture_def.material.color = primary_color ? red : blue;
    primary_color = !primary_color;

    track_body->CreateFixture(&fixture_def);
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

SceneUi::SceneUi(Scene* scene) : scene_(scene) {
  generateControlPoints();
  updateSplines();
}

bool SceneUi::keyPressed(int key) const {
  const auto it = key_state_.find(key);
  return it != key_state_.end() ? it->second : false;
}

void SceneUi::renderOutline(QPainter& painter,
                            const QPen& pen,
                            const math::Outline& outline) const {
  QPainterPath outline_path;
  auto& nodes = outline.nodes();
  for (size_t i = 0; i < nodes.size(); ++i) {
    if (i == 0) {
      const auto point = nodes.back().p;
      outline_path.moveTo(point.x, point.y);
    }
    const auto point = nodes[i].p;
    outline_path.lineTo(point.x, point.y);
  }
  painter.setPen(pen);
  painter.setBrush(Qt::NoBrush);
  painter.drawPath(outline_path);
}

void SceneUi::renderSegments(QPainter& painter,
                             const math::Outline& outline,
                             float track_width) const {
  painter.setBrush(Qt::NoBrush);
  painter.setPen(QPen(Qt::gray, 0));
  for (const auto& node : outline.nodes()) {
    const auto start = node.p;
    const auto end = node.offset(-track_width);
    painter.drawLine(QLineF(start.x, start.y, end.x, end.y));
  }
}

void SceneUi::renderControlPoints(QPainter& painter,
                                  const QColor& color,
                                  const math::Polygon& control_points) const {
  painter.setBrush(color);
  QPainterPath cp_path;
  const auto& last_cp = control_points.back();
  cp_path.moveTo(last_cp.x, last_cp.y);
  for (const auto& cp : control_points) {
    painter.setPen(QPen(color, 0));
    painter.drawEllipse(QPointF(cp.x, cp.y), 0.05, 0.05);
    cp_path.lineTo(cp.x, cp.y);
    painter.setPen(QPen(Qt::lightGray, 0, Qt::DotLine));
    painter.drawLine(QLineF(0, 0, cp.x, cp.y));
  }
  painter.setBrush(Qt::NoBrush);
  painter.setPen(QPen(color, 0));
  painter.drawPath(cp_path);
}

void SceneUi::render(QPainter& painter, const QRectF&, bool) {
  if (!control_points_.empty()) {
    renderControlPoints(painter, Qt::green, control_points_);
    if (render_outer_control_points_) {
      renderControlPoints(painter, Qt::cyan, outer_control_points_);
    }
  }

  if (!inner_outline_.empty()) {
    renderOutline(painter, QPen(Qt::blue, 0, Qt::DashLine), inner_outline_);
    renderOutline(painter, QPen(Qt::blue, 0, Qt::DotLine), outer_outline_);
    if (render_segments_) {
      renderSegments(painter, outer_outline_, scene_->config()->track_width);
    }
  }
}

void SceneUi::step() {
  // placeholder
}

void SceneUi::mousePressEvent(const QPointF& pos, QMouseEvent* event) {
  const auto x = float(pos.x());
  const auto y = float(pos.y());

  if ((event->buttons() & Qt::LeftButton) != 0) {
    scene_->clear();
    control_points_.push_back(math::Vector2d(x, y));
    updateSplines();
  }
}

void SceneUi::keyPressEvent(QKeyEvent* event) {
  key_state_[event->key()] = true;

  switch (event->key()) {
    case Qt::Key_N:
      scene_->clear();
      generateControlPoints();
      updateSplines();
      break;
    case Qt::Key_D:
      scene_->clear();
      control_points_.clear();
      updateSplines();
      break;
    case Qt::Key_C:
      scene_->clear();
      create_curbs_ = !create_curbs_;
      updateSplines();
      break;
    case Qt::Key_S:
      render_segments_ = !render_segments_;
      break;
    case Qt::Key_E:
      scene_->clear();
      use_equidistant_outlines_ = !use_equidistant_outlines_;
      updateSplines();
      break;
  }
}

QString SceneUi::help() const {
  return "Left click: insert control point\n"
         "D: clear scene\n"
         "N: generate new spline\n"
         "C: toggle curb objects\n"
         "S: toggle render segments\n"
         "E: toggle equidistant outlines";
}

void SceneUi::generateControlPoints() {
  const auto config = scene_->config();
  const double x_limit = config->width / 2 - config->track_width;
  const double y_limit = config->height / 2 - config->track_width;
  const double radius = (config->width + config->height) / 2.0;
  std::uniform_real_distribution<double> dist(0.1f, radius);
  std::default_random_engine rnd(core::randomSeed());

  control_points_.resize(config->track_complexity);
  for (size_t i = 0; i < config->track_complexity; ++i) {
    const double angle = i * math::kPi * 2 / config->track_complexity;
    const double r = dist(rnd);
    const double x = cos(angle) * r;
    const double y = sin(angle) * r;

    // truncate the point to the track area rectangle
    const double vt = (y >= 0 ? y_limit : -y_limit) / y;
    const double ht = (x >= 0 ? x_limit : -x_limit) / x;
    const double t = min(min(vt, ht), 1.0);
    control_points_[i].x = x * t;
    control_points_[i].y = y * t;
  }
}

void SceneUi::updateSplines() {
  if (control_points_.size() < 3) {
    inner_outline_.clear();
    outer_outline_.clear();
    return;
  }

  const auto config = scene_->config();
  const int resolution = config->track_resolution;
  const float track_width = config->track_width;

  // create the inner spline
  inner_outline_ = math::Outline(control_points_, resolution);

  // create the outer spline
  outer_control_points_ = inner_outline_.offset(track_width).toPolygon();
  outer_outline_ = math::Outline(outer_control_points_, resolution);

  if (use_equidistant_outlines_) {
    inner_outline_ = inner_outline_.makeEquidistant();
    outer_outline_ = outer_outline_.makeEquidistant();
  }

  // create the fixtures
  if (create_curbs_) {
    scene_->createCurb(inner_outline_, -config->curb_width);
    scene_->createCurb(outer_outline_, config->curb_width);
  }
}

}  // namespace splines_scene
