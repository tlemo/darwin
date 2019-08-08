
#pragma once

#include "physics.h"
#include "sandbox_factory.h"

#include <core_ui/box2d_widget.h>

#include <QKeyEvent>

#include <memory>
using namespace std;

namespace sandbox_scene_6 {

class Scene : public phys::Scene {
 public:
  Scene();

  void moveCart(float force);

 private:
  b2Body* cart_ = nullptr;
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

}  // namespace sandbox_scene_6
