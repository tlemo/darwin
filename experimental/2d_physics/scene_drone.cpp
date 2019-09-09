
#include "scene_drone.h"

#include <core/global_initializer.h>
#include <core/math_2d.h>

#include <QPainter>
#include <QRectF>
#include <QPointF>
#include <QPolygonF>

namespace drone_scene {

GLOBAL_INITIALIZER {
  scenesRegistry().add<Factory>("Drone");
}

Scene::Scene(const core::PropertySet* config)
    : phys::Scene(b2Vec2(0, 0), phys::Rect(-10, -10, 20, 20)) {
  if (config) {
    config_.copyFrom(*config);
  }

  // walls
  b2BodyDef walls_def;
  auto walls = world_.CreateBody(&walls_def);

  b2EdgeShape wall_shape;
  wall_shape.Set(b2Vec2(-10, -10), b2Vec2(10, -10));
  walls->CreateFixture(&wall_shape, 0.0f);
  wall_shape.Set(b2Vec2(-10, -10), b2Vec2(-10, 10));
  walls->CreateFixture(&wall_shape, 0.0f);
  wall_shape.Set(b2Vec2(10, -10), b2Vec2(10, 10));
  walls->CreateFixture(&wall_shape, 0.0f);
  wall_shape.Set(b2Vec2(-10, 10), b2Vec2(10, 10));
  walls->CreateFixture(&wall_shape, 0.0f);

  // drone
  b2BodyDef drone_body_def;
  drone_body_def.type = b2_dynamicBody;
  drone_body_def.position.SetZero();
  drone_body_def.linearDamping = 10.0f;
  drone_body_def.angularDamping = 10.0f;
  drone_ = world_.CreateBody(&drone_body_def);

  b2CircleShape drone_shape;
  drone_shape.m_radius = config_.drone_radius;

  b2FixtureDef drone_fixture_def;
  drone_fixture_def.shape = &drone_shape;
  drone_fixture_def.density = 0.1f;
  drone_fixture_def.friction = 1.0f;
  drone_fixture_def.restitution = 0.2f;
  drone_->CreateFixture(&drone_fixture_def);
  
  // camera
  camera_ = make_unique<phys::Camera>(drone_, 0.4f, 0.1f, 1000.0f, 512);
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
  body->CreateFixture(&fixture_def);
}

void Scene::updateVariables() {
  variables_.drone_x = drone_->GetPosition().x;
  variables_.drone_y = drone_->GetPosition().y;
  variables_.drone_vx = drone_->GetLinearVelocity().x;
  variables_.drone_vy = drone_->GetLinearVelocity().y;
  variables_.drone_dir = drone_->GetAngle();
}

void SceneUi::renderCamera(QPainter& painter, const phys::Camera* camera) const {
  auto body = camera->body();
  
  const float near = camera->near();
  const float far = camera->far();
  const float near_x = -camera->width() / 2;
  const float far_x = near_x * (far / near);
  
  const b2Vec2 far_left(-far_x, far);
  const b2Vec2 far_right(far_x, far);
  
  auto vec2point = [](const b2Vec2& vec) { return QPointF(vec.x, vec.y); };
  
  QPolygonF frustum;
  frustum << vec2point(body->GetWorldPoint(b2Vec2(0, 0)));
  frustum << vec2point(body->GetWorldPoint(far_left));
  frustum << vec2point(body->GetWorldPoint(far_right));

  painter.setPen(Qt::NoPen);
  painter.setBrush(QColor(64, 64, 64, 32));  
  painter.drawPolygon(frustum);
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

}  // namespace drone_scene
