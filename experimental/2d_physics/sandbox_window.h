
#pragma once

#include "physics.h"
#include "sandbox_factory.h"

#include <core_ui/box2d_sandbox_window.h>
#include <core_ui/properties_widget.h>

class SandboxWindow : public core_ui::Box2dSandboxWindow {
  struct Variables {
    // simulation state
    core_ui::PropertyItem* state = nullptr;
    core_ui::PropertyItem* step = nullptr;
    core_ui::PropertyItem* timestamp = nullptr;
    core_ui::PropertyItem* objects_count = nullptr;
  };

 public:
  explicit SandboxWindow(SandboxFactory* factory);
  
  void newScene() override;
  void singleStep() override;
  void updateUI() override;
  
  string name() const { return scene_package_.scene->name(); }

 private:
  void setupVariables();

 private:
  Variables variables_;
  
  SandboxFactory* factory_ = nullptr;
  SandboxScenePackage scene_package_;
  int step_ = -1;
};

