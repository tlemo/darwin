
#pragma once

#include "physics.h"
#include "sandbox_factory.h"

#include <core_ui/box2d_widget.h>

#include <memory>
using namespace std;

namespace sandbox_scene_1 {

class Scene : public phys::Scene {
 public:
  Scene();

  string name() const override { return "Scene1"; }
};

class SceneUi : public core_ui::Box2dSceneUi {};

class Factory : public SandboxFactory {
  SandboxScenePackage createScenePackage() override {
    SandboxScenePackage scene_package;
    scene_package.scene = make_unique<Scene>();
    scene_package.scene_ui = make_unique<SceneUi>();
    return scene_package;
  }
};

}  // namespace sandbox_scene_1
