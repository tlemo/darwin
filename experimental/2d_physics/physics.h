
#pragma once

#include "script.h"

#include <core/utils.h>
#include <third_party/box2d/box2d.h>

#include <string>
using namespace std;

namespace phys {

b2Body* addBall(float x, float y, float r, b2World* world);
b2Body* addBox(float x, float y, float w, float h, b2World* world);
b2Body* addCross(float x, float y, float w, float h, b2World* world);
b2Body* addBullet(float x, float y, float dx, float dy, b2World* world);
b2Body* addBoxProjectile(float x, float y, float dx, float dy, b2World* world);

struct Rect {
  float x = 0;
  float y = 0;
  float width = 0;
  float height = 0;

  Rect() = default;

  Rect(float x, float y, float width, float height)
      : x(x), y(y), width(width), height(height) {}
};

class Scene : public core::NonCopyable {
 public:
  Scene(const b2Vec2& gravity, const Rect& extents) : world_(gravity), extents_(extents) {
    script_.start();
  }

  virtual ~Scene() = default;

  const Rect& extents() const { return extents_; }
  
  float timestamp() const { return timestamp_; }
  
  int objectsCount() const { return world_.GetBodyCount(); }
  
  // TODO: temporary workaround, revisit
  // (add support for Scene in Box2dSandboxWindow and Box2dWidget)
  b2World* box2dWorld() { return &world_; }

  virtual string name() const = 0;

  void step() {
    constexpr float32 timeStep = 1.0f / 50.0f;
    constexpr int32 velocityIterations = 10;
    constexpr int32 positionIterations = 10;

    script_.play(timestamp_);

    // Box2D simulation step
    world_.Step(timeStep, velocityIterations, positionIterations);
    timestamp_ += timeStep;

    // TODO: process the contacts
    // TODO: pause/resume/done/reset?
  }
  
 protected:
  b2World world_;
  Rect extents_;
  Script script_;
  float timestamp_ = 0;
};

}  // namespace phys
