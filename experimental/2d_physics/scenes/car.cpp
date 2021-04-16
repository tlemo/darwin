
#include "car.h"

#include <core/global_initializer.h>
#include <core/math_2d.h>

#include <QPainter>
#include <QPointF>
#include <QRectF>

namespace car_scene {

GLOBAL_INITIALIZER {
  scenesRegistry().add<Factory>("Car");
}

Scene::Scene(const core::PropertySet* config)
    : sim::Scene(b2Vec2(0, 0), sim::Rect(-kWidth / 2, -kHeight / 2, kWidth, kHeight)) {
  if (config) {
    config_.copyFrom(*config);
  }

  // walls
  b2BodyDef walls_def;
  auto walls = world_.CreateBody(&walls_def);

  b2EdgeShape wall_shape;
  b2FixtureDef wall_fixture_def;
  wall_fixture_def.shape = &wall_shape;
  wall_fixture_def.friction = 10.0f;
  wall_fixture_def.restitution = 0.5f;
  wall_fixture_def.material.color = b2Color(1, 1, 0);
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

  // lights
  createLight(walls, b2Vec2(-kWidth * 0.4f, 0), b2Color(1, 1, 1));
  createLight(walls, b2Vec2(kWidth * 0.4f, 0), b2Color(1, 1, 1));

  // car
  sim::CarConfig car_config;
  car_config.length = config_.car_length;
  car_config.max_forward_force = config_.max_forward_force;
  car_config.max_reverse_force = config_.max_reverse_force;
  car_config.max_steer_angle = config_.max_steer_angle;
  car_config.tire_traction = config_.tire_traction;
  car_config.camera = true;
  car_config.camera_resolution = 512;
  car_config.camera_fov = 120;
  car_config.touch_sensor = true;
  car_config.touch_resolution = 16;
  car_config.accelerometer = true;
  car_config.compass = true;
  car_config.position = b2Vec2(0, 0);
  car_config.color = b2Color(0, 1, 0);
  car_ = make_unique<sim::Car>(&world_, car_config);

  // dummy car
  car_config.color = b2Color(0, 1, 1);
  car_config.position = b2Vec2(5, 5);
  car_config.angle = float(math::degreesToRadians(-45));
  dummy_car_ = make_unique<sim::Car>(&world_, car_config);

  updateVariables();
}

void Scene::preStep() {
  car_->preStep();
  dummy_car_->preStep();
}

void Scene::postStep(float dt) {
  car_->postStep(dt);
  dummy_car_->postStep(dt);
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
  const b2Body* car_body = car_->body();
  variables_.car_x = car_body->GetPosition().x;
  variables_.car_y = car_body->GetPosition().y;
  variables_.car_velocity = car_->actualVelocity().Length();
  variables_.car_dir = car_body->GetAngle();
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

void SceneUi::render(QPainter& painter, const QRectF&, bool) {
  renderCamera(painter, scene_->camera());
}

void SceneUi::step() {
  const auto config = scene_->config();
  const auto car = scene_->car();

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

}  // namespace car_scene
