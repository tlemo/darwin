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

#pragma once

#include "drone_vision.h"

#include <core/physics/camera.h>
#include <core/physics/scene.h>
#include <core/properties.h>

namespace drone_vision {

using physics::Camera;

struct SceneVariables : public core::PropertySet {
  PROPERTY(drone_x, float, 0, "Drone x coordinate");
  PROPERTY(drone_y, float, 0, "Drone y coordinate");
  PROPERTY(drone_vx, float, 0, "Drone velocity (x component)");
  PROPERTY(drone_vy, float, 0, "Drone velocity (y component)");
  PROPERTY(drone_dir, float, 0, "Heading angle");
};

class Scene : public physics::Scene {
 public:
  explicit Scene(const b2Vec2& target_velocity, const DroneVision* domain);

  const SceneVariables* variables() const override { return &variables_; }

  const Config* config() const override { return nullptr; }

  const Camera* camera() const override { return camera_.get(); }

  void postStep(float dt) override;

  void moveDrone(const b2Vec2& force);
  void rotateDrone(float torque);

  //! returns the angle between the drone direction and the target
  float aimAngle() const;

  const DroneVision* domain() const { return domain_; }

 private:
  b2Body* createDrone(const b2Vec2& pos, float radius);
  b2Body* createTarget(const b2Vec2& pos, const b2Vec2& v, float radius);
  void createLight(b2Body* body, const b2Vec2& pos, const b2Color& color);
  void updateVariables();

 private:
  b2Body* drone_ = nullptr;
  b2Body* target_ = nullptr;
  unique_ptr<Camera> camera_;
  SceneVariables variables_;
  const DroneVision* domain_ = nullptr;
};

}  // namespace drone_vision
