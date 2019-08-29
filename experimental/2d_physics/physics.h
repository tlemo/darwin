
#pragma once

#include "script.h"

#include <core/properties.h>
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
  virtual void postStep() {}
  virtual void onContact(b2Contact* /*contact*/) {}

  // TODO: temporary workaround, revisit
  // (add support for Scene in Box2dSandboxWindow and Box2dWidget)
  b2World* box2dWorld() { return &world_; }

  void step();

 protected:
  b2World world_;
  Rect extents_;
  Script script_;
  float timestamp_ = 0;
  ContactListener contact_listener_;
};

}  // namespace phys
