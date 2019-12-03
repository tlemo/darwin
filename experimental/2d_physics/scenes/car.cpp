
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
  wall_fixture_def.friction = 1.0f;
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

  // drone
  drone_ = createDrone(b2Vec2(0, 0), config_.drone_radius);

  // dummy drone
  createDrone(b2Vec2(0, 5), config_.drone_radius);

  // lights
  createLight(walls, b2Vec2(-9, -9), b2Color(1, 1, 1));
  createLight(walls, b2Vec2(9, 9), b2Color(1, 1, 1));

  // sensors
  camera_ = make_unique<Camera>(drone_, 120, 0.1f, 30.0f, 512);
  touch_sensor_ = make_unique<TouchSensor>(drone_, 16);
  accelerometer_ = make_unique<Accelerometer>(drone_);
  compass_ = make_unique<Compass>(drone_);
  
  updateVariables();
}

void Scene::postStep(float dt) {
  accelerometer_->update(dt);
  updateVariables();
}

void Scene::moveDrone(const b2Vec2& force) {
  drone_->ApplyForceToCenter(drone_->GetWorldVector(force), true);
}

void Scene::rotateDrone(float torque) {
  drone_->ApplyTorque(torque, true);
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

b2Body* Scene::createDrone(const b2Vec2& pos, float radius) {
  b2BodyDef drone_body_def;
  drone_body_def.type = b2_dynamicBody;
  drone_body_def.position = pos;
  drone_body_def.linearDamping = 10.0f;
  drone_body_def.angularDamping = 10.0f;
  auto body = world_.CreateBody(&drone_body_def);

  b2CircleShape drone_shape;
  drone_shape.m_radius = radius;

  b2FixtureDef drone_fixture_def;
  drone_fixture_def.shape = &drone_shape;
  drone_fixture_def.density = 0.1f;
  drone_fixture_def.friction = 1.0f;
  drone_fixture_def.restitution = 0.2f;
  drone_fixture_def.material.color = b2Color(0, 0, 1);
  drone_fixture_def.material.emit_intensity = 0.5f;
  body->CreateFixture(&drone_fixture_def);

  // left light
  b2CircleShape left_light_shape;
  left_light_shape.m_radius = radius / 4;
  left_light_shape.m_p = b2Vec2(-radius, 0);

  b2FixtureDef left_light_def;
  left_light_def.shape = &left_light_shape;
  left_light_def.material.color = b2Color(1, 0, 0);
  left_light_def.material.emit_intensity = 1;
  left_light_def.filter.maskBits = 0;
  body->CreateFixture(&left_light_def);

  // right light
  b2CircleShape right_light_shape;
  right_light_shape.m_radius = radius / 4;
  right_light_shape.m_p = b2Vec2(radius, 0);

  b2FixtureDef right_light_def;
  right_light_def.shape = &right_light_shape;
  right_light_def.material.color = b2Color(0, 1, 0);
  right_light_def.material.emit_intensity = 1;
  right_light_def.filter.maskBits = 0;
  body->CreateFixture(&right_light_def);

  return body;
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
  variables_.drone_x = drone_->GetPosition().x;
  variables_.drone_y = drone_->GetPosition().y;
  variables_.drone_vx = drone_->GetLinearVelocity().x;
  variables_.drone_vy = drone_->GetLinearVelocity().y;
  variables_.drone_dir = drone_->GetAngle();
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

void SceneUi::renderDrone(QPainter& painter) const {
  auto vars = scene_->variables();
  auto config = scene_->config();
  const float radius = config->drone_radius;
  painter.save();
  painter.translate(vars->drone_x, vars->drone_y);
  painter.scale(1, -1);
  painter.rotate(math::radiansToDegrees(-vars->drone_dir));
  const QRectF dest_rect(-radius, -radius, radius * 2, radius * 2);
  painter.drawPixmap(dest_rect, drone_pixmap_, drone_pixmap_.rect());
  painter.restore();
}

void SceneUi::render(QPainter& painter, const QRectF&) {
  renderDrone(painter);
  renderCamera(painter, scene_->camera());
}

void SceneUi::step() {
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
