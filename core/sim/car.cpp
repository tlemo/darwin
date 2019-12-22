// Copyright 2019 The Darwin Neuroevolution Framework Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "car.h"

#include <core/math_2d.h>

#include <math.h>

namespace sim {

Car::Car(b2World* world, const CarConfig& config) : config_(config) {
  CHECK(config_.max_forward_force >= 0);
  CHECK(config_.max_reverse_force >= 0);

  last_position_ = config_.position;

  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  body_def.position = config_.position;
  body_def.angle = config_.angle;
  body_def.linearDamping = 10.0f;
  body_def.angularDamping = 50.0f;
  car_body_ = world->CreateBody(&body_def);

  const float dx = width() / 2;
  const float dy = config_.length / 2;

  // main frame fixture
  b2PolygonShape shape;
  shape.SetAsBox(dx, dy);

  b2FixtureDef fixture_def;
  fixture_def.shape = &shape;
  fixture_def.density = config_.density;
  fixture_def.friction = config_.friction;
  fixture_def.restitution = 0.1f;
  fixture_def.material.color = config_.color;
  fixture_def.material.shininess = 25;
  fixture_def.material.emit_intensity = 0.1f;
  fixture_def.userData = car_body_;
  car_body_->CreateFixture(&fixture_def);

  // create fixed wheels
  const float rear_axle_dy = rearAxleOffset();
  createWheelFixture(car_body_, b2Vec2(-dx, rear_axle_dy));
  createWheelFixture(car_body_, b2Vec2(dx, rear_axle_dy));

  // create car lights
  const float light_dx = dx * 0.7f;
  const b2Color red(1, 0, 0);
  const b2Color white(1, 1, 1);
  createLightFixture(b2Vec2(-light_dx, -dy), red);
  createLightFixture(b2Vec2(light_dx, -dy), red);
  createLightFixture(b2Vec2(-light_dx, dy), white);
  createLightFixture(b2Vec2(light_dx, dy), white);

  // create the front (turning) wheels
  const float front_axle_dy = frontAxleOffset();
  left_wheel_joint_ = createTurningWheel(world, b2Vec2(-dx, front_axle_dy));
  right_wheel_joint_ = createTurningWheel(world, b2Vec2(dx, front_axle_dy));

  createSensors();
}

void Car::createSensors() {
  if (config_.camera) {
    camera_ = make_unique<Camera>(
        car_body_, config_.camera_fov, 0.1f, 50.0f, config_.camera_resolution);
    camera_->setPosition(b2Vec2(0, config_.length * 0.2f));
    camera_->setFilterId(car_body_);
  }

  if (config_.touch_sensor) {
    touch_sensor_ = make_unique<TouchSensor>(car_body_, config_.touch_resolution);
  }

  if (config_.accelerometer) {
    accelerometer_ = make_unique<Accelerometer>(car_body_);
  }

  if (config_.compass) {
    compass_ = make_unique<Compass>(car_body_);
  }
}

void Car::createWheelFixture(b2Body* body, const b2Vec2& pos) {
  const float wheel_size = config_.length * 0.1f;

  b2PolygonShape shape;
  shape.SetAsBox(wheel_size / 2, wheel_size, pos, 0);

  b2FixtureDef fixture_def;
  fixture_def.shape = &shape;
  fixture_def.density = config_.density;
  fixture_def.friction = config_.friction;
  fixture_def.restitution = 0.1f;
  fixture_def.material.color = b2Color(0.2f, 0.2f, 0.2f);
  fixture_def.material.shininess = 0;
  fixture_def.filter.maskBits = 0;
  fixture_def.userData = car_body_;
  body->CreateFixture(&fixture_def);
}

void Car::createLightFixture(const b2Vec2& pos, const b2Color& color) {
  b2CircleShape shape;
  shape.m_radius = config_.length * 0.04f;
  shape.m_p = pos;

  b2FixtureDef fixture_def;
  fixture_def.shape = &shape;
  fixture_def.density = config_.density;
  fixture_def.friction = config_.friction;
  fixture_def.restitution = 0.1f;
  fixture_def.material.color = color;
  fixture_def.material.shininess = 10;
  fixture_def.material.emit_intensity = 1.0f;
  fixture_def.userData = car_body_;
  car_body_->CreateFixture(&fixture_def);
}

b2RevoluteJoint* Car::createTurningWheel(b2World* world, const b2Vec2& pos) {
  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  body_def.position = car_body_->GetWorldPoint(pos);
  body_def.angularDamping = 10.0f;
  auto body = world->CreateBody(&body_def);

  createWheelFixture(body, b2Vec2(0, 0));

  b2RevoluteJointDef hinge_def;
  hinge_def.bodyA = car_body_;
  hinge_def.bodyB = body;
  hinge_def.localAnchorA = pos;
  hinge_def.localAnchorB.Set(0.0f, 0.0f);
  hinge_def.enableLimit = true;
  hinge_def.lowerAngle = 0;
  hinge_def.upperAngle = 0;
  return static_cast<b2RevoluteJoint*>(world->CreateJoint(&hinge_def));
}

void Car::updateSteering() {
  // get the target steer angle (in radians)
  const float max_steer_angle = float(math::degreesToRadians(config_.max_steer_angle));
  const float target_angle = -target_steer_ * max_steer_angle;

  // current steering angle
  const float current_angle = left_wheel_joint_->GetJointAngle();

  // gradually steer towards the target angle
  const float new_angle = current_angle + (target_angle - current_angle) * 0.2f;
  left_wheel_joint_->SetLimits(new_angle, new_angle);
  right_wheel_joint_->SetLimits(new_angle, new_angle);
}

void Car::applyBrakeImpulse(float intensity,
                            const b2Vec2& wheel_normal,
                            const b2Vec2& wheel_center) {
  CHECK(intensity >= 0 && intensity <= 1);

  const auto wheel_dir = wheel_normal.Skew();
  const auto rolling_velocity = b2Dot(wheel_dir, actual_velocity_) * wheel_dir;

  auto impulse = car_body_->GetMass() * -rolling_velocity;
  const auto impulse_length = impulse.Length();
  if (impulse_length > config_.tire_traction) {
    impulse *= config_.tire_traction / impulse_length;
  }
  impulse *= intensity;

  car_body_->ApplyLinearImpulse(impulse, wheel_center, true);
}

void Car::applyTireLateralImpulse(const b2Vec2& wheel_normal,
                                  const b2Vec2& wheel_center) {
  const auto lateral_velocity = b2Dot(wheel_normal, actual_velocity_) * wheel_normal;
  const auto mass = car_body_->GetMass();
  const auto max_impulse = mass * config_.tire_traction;

  auto impulse = mass * -lateral_velocity;
  const auto impulse_length = impulse.Length();
  if (impulse_length > max_impulse) {
    impulse *= max_impulse / impulse_length;
  }

  car_body_->ApplyLinearImpulse(impulse, wheel_center, true);
}

void Car::preStep() {
  const b2Body* left_wheel_body = left_wheel_joint_->GetBodyB();
  const auto front_wheel_normal = left_wheel_body->GetWorldVector(b2Vec2(1, 0));
  const auto rear_wheel_normal = car_body_->GetWorldVector(b2Vec2(1, 0));
  const auto front_wheel_center = car_body_->GetWorldPoint(b2Vec2(0, frontAxleOffset()));
  const auto rear_wheel_center = car_body_->GetWorldPoint(b2Vec2(0, rearAxleOffset()));

  // apply brakes
  applyBrakeImpulse(brake_pedal_, rear_wheel_normal, rear_wheel_center);
  applyBrakeImpulse(brake_pedal_, front_wheel_normal, front_wheel_center);

  updateSteering();

  // wheel(s) lateral traction
  applyTireLateralImpulse(rear_wheel_normal, rear_wheel_center);
  applyTireLateralImpulse(front_wheel_normal, front_wheel_center);
}

void Car::postStep(float dt) {
  // update position and actual velocity
  const auto new_position = car_body_->GetPosition();
  actual_velocity_ = (new_position - last_position_) * (1 / dt);
  last_position_ = new_position;

  // update accelerometer
  if (accelerometer_ != nullptr) {
    accelerometer_->update(dt);
  }

  // reset brakes
  brake_pedal_ = 0;
}

void Car::accelerate(float force) {
  if (force > config_.max_forward_force) {
    force = config_.max_forward_force;
  }
  if (force < -config_.max_reverse_force) {
    force = -config_.max_reverse_force;
  }
  car_body_->ApplyForceToCenter(car_body_->GetWorldVector(b2Vec2(0, force)), true);
}

void Car::steer(float steer_wheel_position) {
  // clamp the target steer to [-1, +1]
  target_steer_ = min(max(steer_wheel_position, -1.0f), 1.0f);
}

void Car::brake(float intensity) {
  // clamp the braking intensity to [0, 1]
  brake_pedal_ = min(max(intensity, 0.0f), 1.0f);
}

}  // namespace sim
