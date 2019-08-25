
#pragma once

#include "physics.h"
#include "sandbox_factory.h"

#include <core/properties.h>
#include <core_ui/box2d_widget.h>

#include <QKeyEvent>

#include <memory>
using namespace std;

namespace cart_pole_scene {

struct SceneVariables : public core::PropertySet {
  PROPERTY(cart_distance, float, 0, "Cart distance from the center");
  PROPERTY(cart_velocity, float, 0, "Cart horizontal velocity");
  PROPERTY(pole_angle, float, 0, "Pole angle (from vertical)");
  PROPERTY(pole_angular_velocity, float, 0, "Pole angular velocity");
};

class Scene : public phys::Scene {
 public:
  Scene();

  const SceneVariables* variables() const override { return &variables_; }

  void postStep() override { updateVariables(); }

  void moveCart(float force);

 private:
  void updateVariables();

 private:
  b2Body* cart_ = nullptr;
  b2Body* pole_ = nullptr;
  SceneVariables variables_;
};

class SceneUi : public core_ui::Box2dSceneUi {
  static constexpr float kCartForce = 10.0f;

 public:
  explicit SceneUi(Scene* scene) : scene_(scene) {}

  void keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
      case Qt::Key_Left:
        scene_->moveCart(-kCartForce);
        break;
      case Qt::Key_Right:
        scene_->moveCart(kCartForce);
        break;
    }
  }

 private:
  Scene* scene_ = nullptr;
};

class Factory : public SandboxFactory {
  SandboxScenePackage createScenePackage() override {
    auto scene = make_unique<Scene>();
    auto scene_ui = make_unique<SceneUi>(scene.get());
    return { std::move(scene), std::move(scene_ui) };
  }
};

}  // namespace cart_pole_scene
