
#include "drone_race.h"
#include "sandbox_factory.h"

#include <core/math_2d.h>
#include <core/global_initializer.h>

#include <QPainter>
#include <QPointF>
#include <QLineF>
#include <QRectF>
#include <QBrush>
#include <QPen>
#include <QColor>
#include <QPainterPath>

#include <random>
using namespace std;

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
  track_ = createTrack();
  track_->createFixtures(&world_);
  drone_ = createDrone();
  updateVariables();
}

void Scene::postStep(float dt) {
  drone_->postStep(dt);
  track_distance_ =
      track_->updateTrackDistance(track_distance_, drone_->body()->GetPosition());
  updateVariables();
}

void Scene::moveDrone(const b2Vec2& force) {
  drone_->move(force);
}

void Scene::rotateDrone(float torque) {
  drone_->rotate(torque);
}

static b2Vec2 toBox2dVec(const math::Vector2d& v) {
  return b2Vec2(float(v.x), float(v.y));
}

unique_ptr<sim::Drone> Scene::createDrone() {
  // calculate the start position
  const auto start_node = track_->distanceToNode(0);
  const auto start_pos = start_node.offset(-config_.track_width / 2);
  const auto start_angle = atan2(start_node.n.y, start_node.n.x);

  sim::DroneConfig drone_config;
  drone_config.position = toBox2dVec(start_pos);
  drone_config.angle = float(start_angle);
  drone_config.radius = config_.drone_radius;
  drone_config.camera = true;
  drone_config.camera_resolution = 256;
  drone_config.touch_sensor = true;
  drone_config.accelerometer = true;
  drone_config.compass = true;
  drone_config.max_move_force = config_.move_force;
  drone_config.max_lateral_force = config_.lateral_force;
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

unique_ptr<sim::Track> Scene::createTrack() {
  sim::TrackConfig track_config;
  track_config.width = config_.track_width;
  track_config.complexity = config_.track_complexity;
  track_config.resolution = config_.track_resolution;
  track_config.area_width = kWidth;
  track_config.area_height = kHeight;
  track_config.curb_width = config_.curb_width;
  track_config.curb_friction = config_.curb_friction;
  track_config.gates = config_.track_gates;
  track_config.solid_gate_posts = config_.solid_gate_posts;
  const auto random_seed = std::random_device{}();
  return make_unique<sim::Track>(random_seed, track_config);
}

void Scene::updateVariables() {
  variables_.drone_x = drone_->body()->GetPosition().x;
  variables_.drone_y = drone_->body()->GetPosition().y;
  variables_.drone_vx = drone_->body()->GetLinearVelocity().x;
  variables_.drone_vy = drone_->body()->GetLinearVelocity().y;
  variables_.drone_dir = drone_->body()->GetAngle();
  variables_.track_distance = track_distance_;
}

void SceneUi::renderCamera(QPainter& painter, const sim::Camera* camera) const {
  auto body = camera->body();
  const float far = camera->far();
  const float fov = camera->fov();
  const auto pos = body->GetWorldPoint(camera->position());

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

void SceneUi::renderTrack(QPainter& painter) const {
  QPainterPath track_path;

  // inner track edge
  const auto& inner_nodes = scene_->track()->innerOutline().nodes();
  for (size_t i = 0; i < inner_nodes.size(); ++i) {
    const auto& node = inner_nodes[i];
    if (i == 0) {
      track_path.moveTo(node.p.x, node.p.y);
    } else {
      track_path.lineTo(node.p.x, node.p.y);
    }
  }

  // outer track edge
  const auto& outer_nodes = scene_->track()->outerOutline().nodes();
  for (size_t i = 0; i < outer_nodes.size(); ++i) {
    const auto& node = outer_nodes[i];
    if (i == 0) {
      track_path.moveTo(node.p.x, node.p.y);
    } else {
      track_path.lineTo(node.p.x, node.p.y);
    }
  }

  painter.setPen(Qt::NoPen);
  painter.setBrush(QColor(240, 240, 240));
  painter.drawPath(track_path);
}

void SceneUi::renderCurrentSegment(QPainter& painter) const {
  const auto track = scene_->track();
  constexpr float kOffset = 0.4f;
  const auto vars = scene_->variables();
  const auto& node = track->distanceToNode(vars->track_distance);
  const auto p1 = node.offset(kOffset);
  const auto p2 = node.offset(-scene_->config()->track_width - kOffset);
  painter.setBrush(Qt::NoBrush);
  painter.setPen(QPen(Qt::green, 0));
  painter.drawLine(QPointF(p1.x, p1.y), QPointF(p2.x, p2.y));
}

SceneUi::SceneUi(Scene* scene) : scene_(scene) {
  const auto vars = scene_->variables();
  drone_path_.moveTo(vars->drone_x, vars->drone_y);
}

bool SceneUi::keyPressed(int key) const {
  const auto it = key_state_.find(key);
  return it != key_state_.end() ? it->second : false;
}

void SceneUi::render(QPainter& painter, const QRectF& viewport, bool /*debug*/) {
  painter.setPen(Qt::NoPen);
  painter.setBrush(Qt::white);
  painter.drawRect(viewport);

  renderTrack(painter);
  renderPath(painter);
  renderCurrentSegment(painter);
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
  const float lateral_force = scene_->config()->lateral_force;
  const float rotate_torque = scene_->config()->rotate_torque;
  if (keyPressed(Qt::Key_Left)) {
    scene_->moveDrone(b2Vec2(-lateral_force, 0));
  }
  if (keyPressed(Qt::Key_Right)) {
    scene_->moveDrone(b2Vec2(lateral_force, 0));
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
