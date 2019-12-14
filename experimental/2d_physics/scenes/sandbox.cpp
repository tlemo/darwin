
#include "sandbox.h"

#include <core/global_initializer.h>
#include <core/math_2d.h>

#include <QPainter>
#include <QBrush>
#include <QPen>
#include <QPointF>
#include <QRectF>

namespace sandbox_scene {

GLOBAL_INITIALIZER {
  scenesRegistry().add<Factory>("Sandbox");
}

Scene::Scene(const core::PropertySet* config) : sim::Scene(b2Vec2(0, 0), sim::Rect()) {
  if (config) {
    config_.copyFrom(*config);
  }

  const auto width = config_.width;
  const auto height = config_.height;

  setExtents(sim::Rect(-width / 2, -height / 2, width, height));

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

  const b2Vec2 top_left(-width / 2, height / 2);
  const b2Vec2 top_right(width / 2, height / 2);
  const b2Vec2 bottom_left(-width / 2, -height / 2);
  const b2Vec2 bottom_right(width / 2, -height / 2);

  wall_shape.Set(bottom_left, bottom_right);
  walls->CreateFixture(&wall_fixture_def);
  wall_shape.Set(bottom_left, top_left);
  walls->CreateFixture(&wall_fixture_def);
  wall_shape.Set(bottom_right, top_right);
  walls->CreateFixture(&wall_fixture_def);
  wall_shape.Set(top_left, top_right);
  walls->CreateFixture(&wall_fixture_def);

  // lights
  createLight(walls, b2Vec2(-width * 0.4f, 0), b2Color(1, 1, 1));
  createLight(walls, b2Vec2(width * 0.4f, 0), b2Color(1, 1, 1));

  updateVariables();
}

void Scene::preStep() {
  // placeholder
}

void Scene::postStep(float /*dt*/) {
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

void Scene::clear() {
  auto body = world_.GetBodyList();
  while (body != nullptr) {
    auto next = body->GetNext();
    world_.DestroyBody(body);
    body = next;
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

void SceneUi::render(QPainter& painter, const QRectF&) {
  painter.setPen(QPen(Qt::blue, 0));
}

void SceneUi::step() {
  if (keyPressed(Qt::Key_D)) {
    scene_->clear();
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

}  // namespace sandbox_scene
