
#pragma once

#include "physics.h"
#include "sandbox_factory.h"

#include <core_ui/box2d_widget.h>

#include <memory>
using namespace std;

namespace sandbox_scene_6 {

class Scene : public phys::Scene {
 public:
  Scene();
};

class Factory : public SandboxFactory {
  SandboxScenePackage createScenePackage() override {
    return { make_unique<Scene>(), nullptr };
  }
};

}  // namespace sandbox_scene_6
