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

#include "drone.h"

#include <math.h>

namespace sim {

Drone::Drone(b2World* world, const DroneConfig& config) : config_(config) {
  b2BodyDef drone_body_def;
  drone_body_def.type = b2_dynamicBody;
  drone_body_def.position = config_.position;
  drone_body_def.angle = config_.angle;
  drone_body_def.linearDamping = 10.0f;
  drone_body_def.angularDamping = 10.0f;
  drone_body_def.useDefaultRendering = false;
  body_ = world->CreateBody(&drone_body_def);

  b2CircleShape drone_shape;
  drone_shape.m_radius = config_.radius;

  b2FixtureDef drone_fixture_def;
  drone_fixture_def.shape = &drone_shape;
  drone_fixture_def.density = config.density;
  drone_fixture_def.friction = config.friction;
  drone_fixture_def.restitution = config.restitution;
  drone_fixture_def.material.color = config_.color;
  drone_fixture_def.material.emit_intensity = 0.5f;
  drone_fixture_def.material.shininess = 10;
  body_->CreateFixture(&drone_fixture_def);

  if (config.lights) {
    // left light
    b2CircleShape left_light_shape;
    left_light_shape.m_radius = config_.radius / 4;
    left_light_shape.m_p = b2Vec2(-config_.radius, 0);

    b2FixtureDef left_light_def;
    left_light_def.shape = &left_light_shape;
    left_light_def.material.color = b2Color(1, 0, 0);
    left_light_def.material.emit_intensity = 1;
    left_light_def.filter.maskBits = 0;
    body_->CreateFixture(&left_light_def);

    // right light
    b2CircleShape right_light_shape;
    right_light_shape.m_radius = config_.radius / 4;
    right_light_shape.m_p = b2Vec2(config_.radius, 0);

    b2FixtureDef right_light_def;
    right_light_def.shape = &right_light_shape;
    right_light_def.material.color = b2Color(0, 1, 0);
    right_light_def.material.emit_intensity = 1;
    right_light_def.filter.maskBits = 0;
    body_->CreateFixture(&right_light_def);
  }

  if (config_.camera) {
    camera_ = make_unique<Camera>(
        body_, config_.camera_fov, 0.1f, 30.0f, config_.camera_resolution);
  }

  if (config_.touch_sensor) {
    touch_sensor_ = make_unique<TouchSensor>(body_, config_.touch_resolution);
  }

  if (config_.accelerometer) {
    accelerometer_ = make_unique<Accelerometer>(body_);
  }

  if (config_.compass) {
    compass_ = make_unique<Compass>(body_);
  }
}

void Drone::move(b2Vec2 force) {
  // limit the magnitude of the force
  const float max_x_force = config_.max_lateral_force;
  const float max_y_force = config_.max_move_force;
  CHECK(max_x_force >= 0);
  CHECK(max_y_force >= 0);
  force.x = fminf(force.x, max_x_force);
  force.x = fmaxf(force.x, -max_x_force);
  force.y = fminf(force.y, max_y_force);
  force.y = fmaxf(force.y, -max_y_force);
  body_->ApplyForceToCenter(body_->GetWorldVector(force), true);
}

void Drone::rotate(float torque) {
  // limit the torque
  if (torque < -config_.max_rotate_torque) {
    torque = -config_.max_rotate_torque;
  } else if (torque > config_.max_rotate_torque) {
    torque = config_.max_rotate_torque;
  }
  body_->ApplyTorque(torque, true);
}

void Drone::postStep(float dt) {
  if (accelerometer_ != nullptr) {
    accelerometer_->update(dt);
  }
}

}  // namespace sim
