
#include "car_race.h"
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

namespace car_race_scene {

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
  scenesRegistry().add<Factory>("Car Race");
}

Scene::Scene(const core::PropertySet* config)
    : sim::Scene(b2Vec2(0, 0), sim::Rect(-kWidth / 2, -kHeight / 2, kWidth, kHeight)) {
  if (config) {
    config_.copyFrom(*config);
  }
  track_ = createTrack();
  track_->createFixtures(&world_);
  car_ = createCar();
  updateVariables();
}

void Scene::preStep() {
  car_->preStep();
}

void Scene::postStep(float dt) {
  car_->postStep(dt);
  track_distance_ =
      track_->updateTrackDistance(track_distance_, car_->body()->GetPosition());
  updateVariables();
}

static b2Vec2 toBox2dVec(const math::Vector2d& v) {
  return b2Vec2(float(v.x), float(v.y));
}

unique_ptr<sim::Car> Scene::createCar() {
  // calculate the start position
  const auto start_node = track_->distanceToNode(0);
  const auto start_pos = start_node.offset(-config_.track_width / 2);
  const auto start_angle = atan2(start_node.n.y, start_node.n.x);

  sim::CarConfig car_config;
  car_config.position = toBox2dVec(start_pos);
  car_config.angle = float(start_angle);
  car_config.length = config_.car_length;
  car_config.max_forward_force = config_.max_forward_force;
  car_config.max_reverse_force = config_.max_reverse_force;
  car_config.max_steer_angle = config_.max_steer_angle;
  car_config.tire_traction = config_.tire_traction;
  car_config.camera = true;
  car_config.camera_resolution = 512;
  car_config.touch_sensor = true;
  car_config.touch_resolution = 16;
  car_config.accelerometer = true;
  car_config.compass = true;
  car_config.color = b2Color(0, 1, 0);
  auto car = make_unique<sim::Car>(&world_, car_config);

  // attach a light to the car's body
  b2LightDef light_def;
  light_def.body = car->body();
  light_def.color = b2Color(1, 1, 1);
  light_def.intensity = 2.0f;
  light_def.attenuation_distance = 8.0f;
  world_.CreateLight(&light_def);

  return car;
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
  const b2Body* car_body = car_->body();
  variables_.car_x = car_body->GetPosition().x;
  variables_.car_y = car_body->GetPosition().y;
  variables_.car_velocity = car_body->GetLinearVelocity().Length();
  variables_.car_dir = car_body->GetAngle();
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

void SceneUi::renderPath(QPainter& painter) const {
  painter.setBrush(Qt::NoBrush);
  painter.setPen(QPen(Qt::darkGray, 0, Qt::DotLine));
  painter.drawPath(car_path_);
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
  car_path_.moveTo(vars->car_x, vars->car_y);
}

bool SceneUi::keyPressed(int key) const {
  const auto it = key_state_.find(key);
  return it != key_state_.end() ? it->second : false;
}

void SceneUi::render(QPainter& painter, const QRectF& viewport) {
  painter.setPen(Qt::NoPen);
  painter.setBrush(Qt::white);
  painter.drawRect(viewport);

  renderTrack(painter);
  renderPath(painter);
  renderCurrentSegment(painter);

  if (auto camera = scene_->car()->camera()) {
    renderCamera(painter, camera);
  }
}

void SceneUi::step() {
  const auto vars = scene_->variables();
  const auto config = scene_->config();
  const auto car = scene_->car();

  // update path
  constexpr double kMinDist = 0.1;
  const QPointF car_pos(vars->car_x, vars->car_y);
  if (QLineF(car_pos, car_path_.currentPosition()).length() > kMinDist) {
    car_path_.lineTo(car_pos);
  }

  // steering
  if (keyPressed(Qt::Key_Left)) {
    car->steer(-1);
  } else if (keyPressed(Qt::Key_Right)) {
    car->steer(1);
  } else {
    car->steer(0);
  }

  // braking
  if (keyPressed(Qt::Key_Space)) {
    car->brake(1.0f);
  }

  // forward/reverse
  if (keyPressed(Qt::Key_Up)) {
    car->accelerate(config->max_forward_force);
  } else if (keyPressed(Qt::Key_Down)) {
    car->accelerate(-config->max_reverse_force);
  }
}

}  // namespace car_race_scene
