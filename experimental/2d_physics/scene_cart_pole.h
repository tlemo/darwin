
#pragma once

#include "sandbox_factory.h"

#include <core/physics/scene.h>
#include <core_ui/physics/box2d_widget.h>
#include <core/properties.h>

#include <QKeyEvent>

#include <memory>
using namespace std;

namespace cart_pole_scene {

struct Config : public core::PropertySet {
  PROPERTY(pole_length, float, 1.5f, "Pole length");
  PROPERTY(impulse, float, 1.0f, "Magnitude of the impulse used to control the cart");
};

struct SceneVariables : public core::PropertySet {
  PROPERTY(cart_distance, float, 0, "Cart distance from the center");
  PROPERTY(cart_velocity, float, 0, "Cart horizontal velocity");
  PROPERTY(pole_angle, float, 0, "Pole angle (from vertical)");
  PROPERTY(pole_angular_velocity, float, 0, "Pole angular velocity");
};

class Scene : public physics::Scene {
 public:
  explicit Scene(const core::PropertySet* config);

  const SceneVariables* variables() const override { return &variables_; }

  const Config* config() const override { return &config_; }

  void postStep(float /*dt*/) override { updateVariables(); }

  void moveCart(float impulse);
  
 private:
  void updateVariables();

 private:
  b2Body* cart_ = nullptr;
  b2Body* pole_ = nullptr;
  SceneVariables variables_;
  Config config_;
};

class SceneUi : public physics_ui::Box2dSceneUi {
  static constexpr float kCartImpulse = 1.0f;

 public:
  explicit SceneUi(Scene* scene) : scene_(scene) {}

  void keyPressEvent(QKeyEvent* event) override {
    const float impulse = scene_->config()->impulse;
    switch (event->key()) {
      case Qt::Key_Left:
        scene_->moveCart(-impulse);
        break;
      case Qt::Key_Right:
        scene_->moveCart(impulse);
        break;
    }
  }

 private:
  Scene* scene_ = nullptr;
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

}  // namespace cart_pole_scene
