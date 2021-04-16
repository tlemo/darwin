
#pragma once

#include "sandbox_factory.h"

#include <core/utils.h>
#include <core/sim/scene.h>
#include <core/properties.h>

#include <QKeyEvent>
#include <QPixmap>

#include <memory>
#include <unordered_map>
using namespace std;

namespace sandbox_scene {

struct Config : public core::PropertySet {
  PROPERTY(width, float, 40.0f, "Sandbox area width");
  PROPERTY(height, float, 20.0f, "Sandbox area height");
};

struct SceneVariables : public core::PropertySet {
  PROPERTY(objects_count, int, 0, "The total number of physics objects (bodies)");
};

class Scene : public sim::Scene {
 public:
  explicit Scene(const core::PropertySet* config);

  const SceneVariables* variables() const override { return &variables_; }

  const Config* config() const override { return &config_; }

  void preStep() override;
  void postStep(float dt) override;

  void addBalloon(float x, float y, float radius);
  void addBox(float x, float y, float sx, float sy);

 private:
  void createLight(b2Body* body, const b2Vec2& pos, const b2Color& color);
  void updateVariables();

 private:
  SceneVariables variables_;
  Config config_;
};

class SceneUi : public physics_ui::Box2dSceneUi {
 public:
  explicit SceneUi(Scene* scene) : scene_(scene) {}

  bool keyPressed(int key) const {
    const auto it = key_state_.find(key);
    return it != key_state_.end() ? it->second : false;
  }

  void render(QPainter& painter, const QRectF&, bool) override;

  void step() override;

  void mousePressEvent(const QPointF& pos, QMouseEvent* event) override;

  void keyPressEvent(QKeyEvent* event) override { key_state_[event->key()] = true; }

  void keyReleaseEvent(QKeyEvent* event) override { key_state_[event->key()] = false; }

  void focusOutEvent() override { key_state_.clear(); }

 private:
  Scene* scene_ = nullptr;
  unordered_map<int, bool> key_state_;
};

class Factory : public SandboxFactory {
  SandboxScenePackage createScenePackage(const core::PropertySet* config) override {
    auto scene = make_unique<Scene>(config);
    auto scene_ui = make_unique<SceneUi>(scene.get());
    return { std::move(scene), std::move(scene_ui) };
  }

  unique_ptr<core::PropertySet> defaultConfig() const override {
    return make_unique<Config>();
  }
};

}  // namespace sandbox_scene
