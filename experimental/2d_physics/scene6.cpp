
#include "scene6.h"

Scene6::Scene6() : Scene(b2Vec2(0, -9.8f), phys::Rect(-5, 0, 10, 5)) {
  // ground
  constexpr float kGroundY = 0.1f;
  constexpr float kMaxDistance = 2.5f;
  b2EdgeShape ground_shape;
  ground_shape.Set(b2Vec2(-kMaxDistance, kGroundY), b2Vec2(kMaxDistance, kGroundY));

  b2BodyDef ground_body_def;
  auto ground_body = world_.CreateBody(&ground_body_def);
  ground_body->CreateFixture(&ground_shape, 0.0f);

  // cart
  constexpr float kCartWidth = 0.2f;
  constexpr float kCartHeight = 0.1f;
  b2PolygonShape cart_shape;
  cart_shape.SetAsBox(kCartWidth, kCartHeight);

  b2BodyDef cart_body_def;
  cart_body_def.type = b2_dynamicBody;
  cart_body_def.position.Set(0.0f, kCartHeight + kGroundY);
  auto cart_body = world_.CreateBody(&cart_body_def);

  b2FixtureDef cart_fixture_def;
  cart_fixture_def.shape = &cart_shape;
  cart_fixture_def.density = 0;
  cart_fixture_def.friction = 0;
  cart_body->CreateFixture(&cart_fixture_def);

  // pole
  constexpr float kPoleWidth = 0.04f;
  constexpr float kPoleHeight = 1.5f;
  b2PolygonShape pole_shape;
  pole_shape.SetAsBox(kPoleWidth, kPoleHeight, b2Vec2(0, kPoleHeight), 0);
  pole_shape.SetAsBox(kPoleWidth, kPoleHeight);

  b2BodyDef pole_body_def;
  pole_body_def.type = b2_dynamicBody;
  pole_body_def.position.Set(0.0f, kCartHeight + kPoleHeight + kGroundY);
  auto pole_body = world_.CreateBody(&pole_body_def);

  b2FixtureDef pole_fixture_def;
  pole_fixture_def.shape = &pole_shape;
  pole_fixture_def.density = 1.0f;
  pole_body->CreateFixture(&pole_fixture_def);

  // hinge
  b2RevoluteJointDef hinge_def;
  hinge_def.bodyA = cart_body;
  hinge_def.bodyB = pole_body;
  hinge_def.localAnchorA.Set(0.0f, 0.0f);
  hinge_def.localAnchorB.Set(0.0f, -kPoleHeight);
  world_.CreateJoint(&hinge_def);
}
