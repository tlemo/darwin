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

#include <core/sim/script.h>
#include <core/properties.h>
#include <core/utils.h>
#include <third_party/box2d/box2d.h>

namespace sim {

class Camera;
class TouchSensor;
class Accelerometer;
class Compass;

struct Rect {
  float x = 0;
  float y = 0;
  float width = 0;
  float height = 0;

  Rect() = default;

  Rect(float x, float y, float width, float height)
      : x(x), y(y), width(width), height(height) {}
};

//! High-level physics scene abstraction
class Scene : public core::NonCopyable {
  class ContactListener : public b2ContactListener {
   public:
    explicit ContactListener(Scene* scene) : scene_(scene) {}
    void BeginContact(b2Contact* contact) override { scene_->onContact(contact); }

   private:
    Scene* scene_ = nullptr;
  };

 public:
  Scene(const b2Vec2& gravity, const Rect& extents)
      : world_(gravity), extents_(extents), contact_listener_(this) {
    world_.SetContactListener(&contact_listener_);
  }

  virtual ~Scene() = default;

  const Rect& extents() const { return extents_; }

  float timestamp() const { return timestamp_; }

  int objectsCount() const { return world_.GetBodyCount(); }

  virtual const core::PropertySet* variables() const { return nullptr; }
  virtual const core::PropertySet* config() const { return nullptr; }
  
  virtual void preStep() {}
  virtual void postStep(float /*dt*/) {}
  virtual void onContact(b2Contact* /*contact*/) {}

  // TODO: temporary workaround, revisit
  // (add support for Scene in Box2dSandboxWindow and Box2dWidget)
  b2World* box2dWorld() { return &world_; }
  
  // sensors
  // TODO: get rid of these methods
  virtual const Camera* camera() const { return nullptr; }
  virtual const sim::TouchSensor* touchSensor() const { return nullptr; }
  virtual const sim::Accelerometer* accelerometer() const { return nullptr; }
  virtual const sim::Compass* compass() const { return nullptr; }

  bool simStep();

 protected:
  b2World world_;
  Rect extents_;
  Script script_;
  float timestamp_ = 0;
  ContactListener contact_listener_;
};

}  // namespace sim
