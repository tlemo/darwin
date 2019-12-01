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

#include "scene.h"

#include <core/math_2d.h>

#include <math.h>

namespace drone_track {

Scene::Scene(Seed seed, const DroneTrack* domain)
    : sim::Scene(b2Vec2(0, 0), sim::Rect(-kWidth / 2, -kHeight / 2, kWidth, kHeight)),
      rnd_(seed),
      domain_(domain) {
  const auto& config = domain_->config();

  // create track
  track_ = make_unique<Track>(seed, &world_, config);

  // calculate the start position
  const auto& track_nodes = track_->trackNodes();
  CHECK(!track_nodes.empty());

  const auto start_node = track_nodes[0];
  const auto start_pos = start_node.offsetPos(config.track_width / 2);
  const auto start_angle = atan2f(start_node.normal.y, start_node.normal.x);

  // create the drone
  sim::DroneConfig drone_config = domain_->droneConfig();
  drone_config.position = start_pos;
  drone_config.angle = start_angle;
  drone_ = make_unique<sim::Drone>(&world_, drone_config);

  // attach a light to the drone's body
  b2LightDef light_def;
  light_def.body = drone_->body();
  light_def.color = b2Color(1, 1, 1);
  light_def.intensity = 2.0f;
  light_def.attenuation_distance = 10.0f;
  world_.CreateLight(&light_def);
}

float Scene::fitness() const {
  return float(distance_) / track_->trackNodes().size();
}

void Scene::postStep(float dt) {
  drone_->postStep(dt);
  distance_ = track_->updateTrackDistance(distance_, drone_->body()->GetPosition());
  updateVariables();
}

void Scene::updateVariables() {
  const b2Body* drone_body = drone_->body();
  variables_.drone_x = drone_body->GetPosition().x;
  variables_.drone_y = drone_body->GetPosition().y;
  variables_.drone_vx = drone_body->GetLinearVelocity().x;
  variables_.drone_vy = drone_body->GetLinearVelocity().y;
  variables_.drone_dir = drone_body->GetAngle();
  variables_.distance = distance_;
}

}  // namespace drone_track
