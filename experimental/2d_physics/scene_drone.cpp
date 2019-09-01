
#include "scene_drone.h"

#include <core/global_initializer.h>
#include <core/math_2d.h>

#include <QPainter>
#include <QRectF>

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
}

void Scene::moveDrone(const b2Vec2& force) {
  drone_->ApplyForceToCenter(drone_->GetWorldVector(force), true);
}

void Scene::rotateDrone(float torque) {
  drone_->ApplyTorque(torque, true);
}

void Scene::updateVariables() {
  variables_.drone_x = drone_->GetPosition().x;
  variables_.drone_y = drone_->GetPosition().y;
  variables_.drone_vx = drone_->GetLinearVelocity().x;
  variables_.drone_vy = drone_->GetLinearVelocity().y;
  variables_.drone_dir = drone_->GetAngle();
}

void SceneUi::render(QPainter& painter, const QRectF&) {
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

}  // namespace drone_scene
