
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
  generateRandomTrack();
  updateSplines();
}

void SceneUi::renderSpline(QPainter& painter,
                           const QPen& pen,
                           const vector<SplinePoint>& spline) const {
  QPainterPath spline_path;
  for (size_t i = 0; i < spline.size(); ++i) {
    if (i == 0) {
      const auto point = spline.back().p;
      spline_path.moveTo(point.x, point.y);
    }
    const auto point = spline[i].p;
    spline_path.lineTo(point.x, point.y);
  }
  painter.setPen(pen);
  painter.setBrush(Qt::NoBrush);
  painter.drawPath(spline_path);
}

void SceneUi::renderControlPoints(QPainter& painter,
                                  const vector<math::Vector2d>& control_points) const {
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
}

static QPointF offsetPoint(const SplinePoint& point, double offset) {
  auto offset_p = point.p + point.n * offset;
  return { offset_p.x, offset_p.y };
}

void SceneUi::renderOutline(QPainter& painter,
                            const QPen& pen,
                            const vector<SplinePoint>& spline,
                            double offset) const {
  QPainterPath outline_path;
  for (size_t i = 0; i < spline.size(); ++i) {
    if (i == 0) {
      outline_path.moveTo(offsetPoint(spline.back(), offset));
    }
    outline_path.lineTo(offsetPoint(spline[i], offset));
  }
  painter.setPen(pen);
  painter.setBrush(Qt::NoBrush);
  painter.drawPath(outline_path);
}

static vector<math::Vector2d> createOuterControlPoints(
    const vector<math::Vector2d>& control_points,
    float track_width) {
  auto outer_control_points = control_points;
  for (auto& cp : outer_control_points) {
    const auto len = cp.length();
    cp = cp * ((len + track_width) / len);
  }
  return outer_control_points;
}

void SceneUi::render(QPainter& painter, const QRectF&) {
  const auto track_width = scene_->config()->track_width;

  if (!control_points_.empty()) {
    const auto outer_control_points =
        createOuterControlPoints(control_points_, track_width);
    renderControlPoints(painter, control_points_);
    renderControlPoints(painter, outer_control_points);
  }

  if (!inner_spline_.empty()) {
    renderSpline(painter, QPen(Qt::blue, 0, Qt::DashLine), inner_spline_);
    renderSpline(painter, QPen(Qt::blue, 0, Qt::DotLine), outer_spline_);
    renderOutline(painter, QPen(Qt::red, 0, Qt::DotLine), inner_spline_, track_width);
  }
}

void SceneUi::step() {
  // placeholder
}

void SceneUi::mousePressEvent(const QPointF& pos, QMouseEvent* event) {
  const auto x = float(pos.x());
  const auto y = float(pos.y());

  if ((event->buttons() & Qt::LeftButton) != 0) {
    control_points_.push_back(math::Vector2d(x, y));
    updateSplines();
  }
}

void SceneUi::keyPressEvent(QKeyEvent* event) {
  key_state_[event->key()] = true;

  switch (event->key()) {
    case Qt::Key_N:
      generateRandomTrack();
      updateSplines();
      break;
    case Qt::Key_D:
      control_points_.clear();
      updateSplines();
      break;
  }
}

void SceneUi::generateRandomTrack() {
  const auto config = scene_->config();
  const double x_limit = config->width / 2 - config->track_width;
  const double y_limit = config->height / 2 - config->track_width;
  const double radius = (config->width + config->height) / 2.0;
  std::uniform_real_distribution<double> dist(0.1f, radius);
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

static vector<SplinePoint> createSpline(const vector<math::Vector2d> control_points,
                                        size_t resolution) {
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
  auto samples = spline.sample(resolution + 1);
  samples.pop_back();

  // create the list of points
  vector<SplinePoint> points(resolution);
  for (size_t i = 0; i < resolution; ++i) {
    points[i].p.x = samples[i * 2 + 0];
    points[i].p.y = samples[i * 2 + 1];
  }

  // calculate the segment normals
  vector<math::Vector2d> sn(resolution);
  for (size_t i = 0; i < resolution; ++i) {
    const auto& start_p = points[i].p;
    const auto& end_p = points[(i + 1) % resolution].p;
    const auto v = end_p - start_p;
    sn[i] = math::Vector2d(v.y, -v.x).normalized();
  }

  // calculate the spline point normals
  // (sized appropriately for an curve offset = 1.0, not normal length = 1.0)
  for (size_t i = 0; i < resolution; ++i) {
    const size_t next_i = (i + 1) % resolution;
    const auto a = (sn[i] + sn[next_i]) * 0.5;
    points[next_i].n = a / (a * a);
  }

  return points;
}

void SceneUi::updateSplines() {
  if (control_points_.size() < 3) {
    inner_spline_.clear();
    outer_spline_.clear();
    return;
  }

  const size_t resolution = scene_->config()->track_resolution;
  const float track_width = scene_->config()->track_width;

  // create the inner spline
  inner_spline_ = createSpline(control_points_, resolution);

  // create the outer spline
  auto outer_control_points = createOuterControlPoints(control_points_, track_width);
  outer_spline_ = createSpline(outer_control_points, resolution);
}

}  // namespace splines_scene
