// Copyright The Darwin Neuroevolution Framework Authors.
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

#include <third_party/box2d/box2d.h>

#include <vector>
using namespace std;

namespace physics {

//! A single color & depth "pixel"
struct Receptor {
  b2Color color;
  float distance = 0;

  Receptor() = default;

  explicit Receptor(const b2Color& color, float distance)
      : color(color), distance(distance) {}
};

//! Raytraced rendering of a 2d world
//! 
//! It produces color and depth channels, with a configurable resolution & FOV
//! 
class Camera {
 public:
  Camera(b2Body* body, float fov, float near, float far, int resolution);

  vector<Receptor> render() const;

  b2Body* body() const { return body_; }

  float fov() const { return fov_; }
  float near() const { return near_; }
  float far() const { return far_; }
  int resolution() const { return resolution_; }

 private:
  Receptor castRay(const b2Vec2& ray_start,
                   const b2Vec2& ray_end,
                   float min_fraction) const;

 private:
  b2Body* body_ = nullptr;

  // field of view (in degrees)
  const float fov_ = 0;

  const float near_ = 0;
  const float far_ = 0;

  const int resolution_ = 0;

  // the shadow attenuation factor
  // [0=completely blocking light .. 1=no shadows]
  float shadow_attenuation_ = 1.0f;

  bool render_specular_ = true;
  
  // technically this should be a property of the scene (world),
  // but modeled here for convenience and extra flexibility
  b2Color ambient_light_{ 0.2f, 0.2f, 0.2f };
};

}  // namespace physics
