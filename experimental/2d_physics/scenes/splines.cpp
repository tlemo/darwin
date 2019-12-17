
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
#include <limits>
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
                           const Polygon& spline) const {
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

static math::Vector2d offsetPoint(const PolygonNode& point, double offset) {
  return point.p + point.n * offset;
}

void SceneUi::renderSegments(QPainter& painter,
                             const Polygon& outer_spline,
                             float track_width) const {
  painter.setBrush(Qt::NoBrush);
  painter.setPen(QPen(Qt::gray, 0));
  for (const auto& pn : outer_spline) {
    const auto end_point = offsetPoint(pn, -track_width);
    painter.drawLine(QLineF(pn.p.x, pn.p.y, end_point.x, end_point.y));
  }
}

void SceneUi::renderControlPoints(QPainter& painter,
                                  const QColor& color,
                                  const vector<math::Vector2d>& control_points) const {
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

void SceneUi::renderOutline(QPainter& painter,
                            const QPen& pen,
                            const Polygon& spline,
                            double offset) const {
  QPainterPath outline_path;
  for (size_t i = 0; i < spline.size(); ++i) {
    if (i == 0) {
      const auto point = offsetPoint(spline.back(), offset);
      outline_path.moveTo(point.x, point.y);
    }
    const auto point = offsetPoint(spline[i], offset);
    outline_path.lineTo(point.x, point.y);
  }
  painter.setPen(pen);
  painter.setBrush(Qt::NoBrush);
  painter.drawPath(outline_path);
}

static Polygon createPolygon(const vector<math::Vector2d>& points) {
  const size_t nodes_count = points.size();
  Polygon polygon(nodes_count);

  // calculate the segment normals
  vector<math::Vector2d> sn(nodes_count);
  for (size_t i = 0; i < nodes_count; ++i) {
    const auto& start_p = points[i];
    const auto& end_p = points[(i + 1) % nodes_count];
    const auto v = end_p - start_p;
    sn[i] = math::Vector2d(v.y, -v.x).normalized();
  }

  // set the polygon points and normals
  // (the normals are sized appropriately for an offset = 1.0, not normal length = 1.0)
  for (size_t i = 0; i < nodes_count; ++i) {
    const size_t next_i = (i + 1) % nodes_count;
    const auto a = (sn[i] + sn[next_i]) * 0.5;
    polygon[next_i].p = points[next_i];
    polygon[next_i].n = a / (a * a);
  }

  return polygon;
}

static Polygon createSpline(const vector<math::Vector2d>& control_points,
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
  vector<math::Vector2d> points(resolution);
  for (size_t i = 0; i < resolution; ++i) {
    points[i].x = samples[i * 2 + 0];
    points[i].y = samples[i * 2 + 1];
  }

  return createPolygon(points);
}

// calculate the intersection between 2 segments
//
// the first segment is A:[a1, a2]
// the second segment is B:[b1, b2]
//
// the first pair value in the return value represents the
// intersection offset on segment A, the second is on segment B
//
// the intersection values are [0..1] - if the intersection is on segment
// 0 corresponding to the segment begin, 1 to the segment end. values < 0
// represent intersection before seg begin, >1 after segment end
//
// numeric_limits<double>::infinity() is returned if segments are parallel
//
// the intersection is the solution to:
//   x = x' => x0 + dx * a = x0' + dx' * a'
//   y = y' => y0 + dy * a = y0' + dy' * a'
//
static pair<double, double> intersect(const math::Vector2d& a1,
                                      const math::Vector2d& a2,
                                      const math::Vector2d& b1,
                                      const math::Vector2d& b2) {
  pair<double, double> result;

  const double x = a1.x;
  const double y = a1.y;
  const double dx = a2.x - x;
  const double dy = a2.y - y;

  const double x2 = b1.x;
  const double y2 = b1.y;
  const double dx2 = b2.x - x2;
  const double dy2 = b2.y - y2;

  const double w = dy * dx2 - dx * dy2;

  if (w == 0) {
    result.first = numeric_limits<double>::infinity();
    result.second = numeric_limits<double>::infinity();
  } else {
    result.first = (dx2 * (y2 - y) - dy2 * (x2 - x)) / w;
    result.second = (dx2 != 0) ? ((x + result.first * dx) - x2) / dx2
                               : ((y + result.first * dy) - y2) / dy2;
  }

  return result;
}

static vector<math::Vector2d> fixSelfIntersections(vector<math::Vector2d> points) {
  while (points.size() > 3) {
    const size_t count = points.size();
    const size_t lookahead = count / 4;
    bool self_intersect = false;
    for (size_t i = 0; i < count && !self_intersect; ++i) {
      const auto& a1 = points[i];
      const auto& a2 = points[(i + 1) % count];
      for (size_t j = 0; j < lookahead; ++j) {
        const auto& b1 = points[(i + 2 + j) % count];
        const auto& b2 = points[(i + 3 + j) % count];
        const auto r = intersect(a1, a2, b1, b2);
        if (r.first > 0 && r.first < 1 && r.second > 0 && r.second < 1) {
          // set point[i + 1] to the intersection point
          points[(i + 1) % count] = a1 + (a2 - a1) * r.first;
          // erase all the points [i + 2 .. i + 2 + j]
          const size_t del_first = (i + 2) % count;
          const size_t del_last = (i + 2 + j) % count;
          const auto it = points.begin();
          if (del_first <= del_last) {
            points.erase(it + del_first, it + del_last + 1);
          } else {
            points.erase(it + del_first, points.end());
            points.erase(it, it + del_last + 1);
          }
          self_intersect = true;
          break;
        }
      }
    }
    if (!self_intersect) {
      break;
    }
  }
  return points;
}

static vector<math::Vector2d> createOuterControlPoints(
    const vector<math::Vector2d>& control_points,
    float track_width,
    int track_resolution) {
  const auto polygon = createSpline(control_points, track_resolution);
  vector<math::Vector2d> outer_control_points;
  for (const auto& pn : polygon) {
    if (pn.n.length() > 1.5) {
      outer_control_points.push_back(pn.p + pn.n.normalized() * track_width * 1.5);
    } else {
      outer_control_points.push_back(offsetPoint(pn, track_width));
    }
  }
  return fixSelfIntersections(outer_control_points);
}

void SceneUi::render(QPainter& painter, const QRectF&) {
  const auto track_width = scene_->config()->track_width;
  const auto track_resolution = scene_->config()->track_resolution;

  if (!control_points_.empty()) {
    renderControlPoints(painter, Qt::green, control_points_);
    if (render_outer_control_points_) {
      const auto outer_control_points =
          createOuterControlPoints(control_points_, track_width, track_resolution);
      renderControlPoints(painter, Qt::cyan, outer_control_points);
    }
  }

  if (!inner_spline_.empty()) {
    renderSpline(painter, QPen(Qt::blue, 0, Qt::DashLine), inner_spline_);
    renderSpline(painter, QPen(Qt::blue, 0, Qt::DotLine), outer_spline_);
    if (render_segments_) {
      renderSegments(painter, outer_spline_, track_width);
    }
    if (render_outline_) {
      renderOutline(painter, QPen(Qt::red, 0, Qt::DotLine), inner_spline_, track_width);
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
      scene_->clear();
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

void SceneUi::updateSplines() {
  if (control_points_.size() < 3) {
    inner_spline_.clear();
    outer_spline_.clear();
    return;
  }

  const int resolution = scene_->config()->track_resolution;
  const float track_width = scene_->config()->track_width;

  // create the inner spline
  inner_spline_ = createSpline(control_points_, resolution);

  // create the outer spline
  auto outer_control_points =
      createOuterControlPoints(control_points_, track_width, resolution);
  outer_spline_ = createSpline(outer_control_points, int(resolution * 1.5));
}

}  // namespace splines_scene
