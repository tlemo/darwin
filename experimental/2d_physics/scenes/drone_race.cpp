
#include "drone_race.h"
#include "sandbox_factory.h"

#include <core/math_2d.h>
#include <core/global_initializer.h>
#include <third_party/tinyspline/tinyspline.h>

#include <QPainter>
#include <QPointF>
#include <QLineF>
#include <QRectF>
#include <QBrush>
#include <QPen>
#include <QPainterPath>

namespace drone_race_scene {

class Factory : public SandboxFactory {
  SandboxScenePackage createScenePackage(const core::PropertySet* config) override {
    auto scene = make_unique<Scene>(config);
    auto scene_ui = make_unique<SceneUi>(scene.get());
    return { std::move(scene), std::move(scene_ui) };
  }

  unique_ptr<core::PropertySet> defaultConfig() const override {
    return make_unique<Config>();
  }
};

GLOBAL_INITIALIZER {
  scenesRegistry().add<Factory>("Drone Race");
}

Scene::Scene(const core::PropertySet* config)
    : sim::Scene(b2Vec2(0, 0), sim::Rect(-kWidth / 2, -kHeight / 2, kWidth, kHeight)) {
  if (config) {
    config_.copyFrom(*config);
  }
  generateRandomTrack();
  createTrackFixtures();
  drone_ = createDrone();
  updateVariables();
}

void Scene::postStep(float dt) {
  drone_->postStep(dt);
  updateVariables();
}

void Scene::moveDrone(const b2Vec2& force) {
  drone_->move(force);
}

void Scene::rotateDrone(float torque) {
  drone_->rotate(torque);
}

unique_ptr<sim::Drone> Scene::createDrone() {
  CHECK(!track_nodes_.empty());
  
  const auto start_node = track_nodes_[0];
  const auto start_pos = start_node.offsetPos(config_.track_width / 2);
  const auto start_angle = atan2f(start_node.normal.y, start_node.normal.x);

  sim::DroneConfig drone_config;
  drone_config.position = start_pos;
  drone_config.angle = start_angle;
  drone_config.radius = config_.drone_radius;
  drone_config.camera = true;
  drone_config.camera_resolution = 256;
  drone_config.touch_sensor = true;
  drone_config.accelerometer = true;
  drone_config.compass = true;
  drone_config.max_move_force = config_.move_force;
  drone_config.max_rotate_torque = config_.rotate_torque;
  drone_config.color = b2Color(0, 0, 1);
  drone_config.density = 0.5f;
  drone_config.friction = 0.1f;
  auto drone = make_unique<sim::Drone>(&world_, drone_config);

  // attach a light to the drone's body
  b2LightDef light_def;
  light_def.body = drone->body();
  light_def.color = b2Color(1, 1, 1);
  light_def.intensity = 2.0f;
  light_def.attenuation_distance = 5.0f;
  world_.CreateLight(&light_def);

  return drone;
}

void Scene::generateRandomTrack() {
  CHECK(track_nodes_.empty());

  // generate random control points (counter-clockwise, around the center)
  std::uniform_real_distribution<double> dist(2, 20);
  const double kLimitX = kWidth / 2 - config_.track_width;
  const double kLimitY = kHeight / 2 - config_.track_width;
  vector<math::Vector2d> control_points(config_.track_complexity);
  for (size_t i = 0; i < control_points.size(); ++i) {
    const double angle = i * math::kPi * 2 / config_.track_complexity;
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
  const size_t samples_count = config_.track_resolution;
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

    track_nodes_[i].normal = (prev_n + next_n) * 0.5;
  }
}

void Scene::createTrackFixtures() {
  CHECK(!track_nodes_.empty());

  b2BodyDef track_body_def;
  auto track_body = world_.CreateBody(&track_body_def);

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
    points[2] = track_nodes_[next_i].offsetPos(-kCurbWidth);
    points[3] = track_nodes_[i].offsetPos(-kCurbWidth);
    shape.Set(points, 4);

    fixture_def.material.color = primary_color ? red : white;
    track_body->CreateFixture(&fixture_def);

    // right side curb
    points[0] = track_nodes_[i].offsetPos(config_.track_width + kCurbWidth);
    points[1] = track_nodes_[next_i].offsetPos(config_.track_width + kCurbWidth);
    points[2] = track_nodes_[next_i].offsetPos(config_.track_width);
    points[3] = track_nodes_[i].offsetPos(config_.track_width);
    shape.Set(points, 4);

    fixture_def.material.color = primary_color ? blue : white;
    track_body->CreateFixture(&fixture_def);

    primary_color = !primary_color;
  }
}

void Scene::updateVariables() {
  variables_.drone_x = drone_->body()->GetPosition().x;
  variables_.drone_y = drone_->body()->GetPosition().y;
  variables_.drone_vx = drone_->body()->GetLinearVelocity().x;
  variables_.drone_vy = drone_->body()->GetLinearVelocity().y;
  variables_.drone_dir = drone_->body()->GetAngle();
}

void SceneUi::renderCamera(QPainter& painter, const sim::Camera* camera) const {
  auto body = camera->body();
  const float far = camera->far();
  const float fov = camera->fov();
  const auto pos = body->GetWorldPoint(b2Vec2(0, 0));

  const QPointF center(pos.x, pos.y);
  const QRectF frustum_rect(center - QPointF(far, far), center + QPointF(far, far));
  const double angle = math::radiansToDegrees(body->GetAngle()) + 90 + fov / 2;

  painter.setPen(Qt::NoPen);
  painter.setBrush(QColor(64, 64, 64, 32));
  painter.drawPie(frustum_rect, int(-angle * 16), int(fov * 16));
}

void SceneUi::renderDrone(QPainter& painter, const sim::Drone* drone) const {
  if (auto camera = drone->camera()) {
    renderCamera(painter, camera);
  }

  const auto& drone_config = drone->config();
  const float radius = drone_config.radius;
  const auto drone_body = drone->body();
  const auto pos = drone_body->GetPosition();
  painter.save();
  painter.translate(pos.x, pos.y);
  painter.scale(1, -1);
  painter.rotate(math::radiansToDegrees(-drone_body->GetAngle()));
  const QRectF dest_rect(-radius, -radius, radius * 2, radius * 2);
  painter.drawPixmap(dest_rect, drone_pixmap_, drone_pixmap_.rect());
  painter.restore();

  const auto& color = drone_config.color;
  painter.setPen(QPen(QColor::fromRgbF(color.r, color.g, color.b), 0, Qt::DotLine));
  painter.drawEllipse(QPointF(pos.x, pos.y), radius, radius);
}

void SceneUi::renderPath(QPainter& painter) const {
  painter.setBrush(Qt::NoBrush);
  painter.setPen(QPen(Qt::darkGray, 0, Qt::DotLine));
  painter.drawPath(drone_path_);
}

SceneUi::SceneUi(Scene* scene) : scene_(scene) {
  const auto vars = scene_->variables();
  drone_path_.moveTo(vars->drone_x, vars->drone_y);
}

bool SceneUi::keyPressed(int key) const {
  const auto it = key_state_.find(key);
  return it != key_state_.end() ? it->second : false;
}

void SceneUi::render(QPainter& painter, const QRectF&) {
  renderPath(painter);
  renderDrone(painter, scene_->drone());
}

void SceneUi::step() {
  const auto vars = scene_->variables();

  // update path
  constexpr double kMinDist = 0.1;
  const QPointF drone_pos(vars->drone_x, vars->drone_y);
  if (QLineF(drone_pos, drone_path_.currentPosition()).length() > kMinDist) {
    drone_path_.lineTo(drone_pos);
  }

  // keyboard inputs
  const float move_force = scene_->config()->move_force;
  const float rotate_torque = scene_->config()->rotate_torque;
  if (keyPressed(Qt::Key_Left)) {
    scene_->moveDrone(b2Vec2(-move_force, 0));
  }
  if (keyPressed(Qt::Key_Right)) {
    scene_->moveDrone(b2Vec2(move_force, 0));
  }
  if (keyPressed(Qt::Key_Up)) {
    scene_->moveDrone(b2Vec2(0, move_force));
  }
  if (keyPressed(Qt::Key_Down)) {
    scene_->moveDrone(b2Vec2(0, -move_force));
  }
  if (keyPressed(Qt::Key_Q)) {
    scene_->rotateDrone(rotate_torque);
  }
  if (keyPressed(Qt::Key_W)) {
    scene_->rotateDrone(-rotate_torque);
  }
}

}  // namespace drone_race_scene
