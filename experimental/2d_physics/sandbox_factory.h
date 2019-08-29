
#pragma once

#include "physics.h"

#include <core/modules.h>
#include <core/properties.h>
#include <core_ui/box2d_widget.h>

#include <memory>
using namespace std;

struct SandboxScenePackage {
  unique_ptr<phys::Scene> scene;
  unique_ptr<core_ui::Box2dSceneUi> scene_ui;
};

class SandboxFactory : public core::ModuleFactory {
 public:
  // config may be nullptr (for no configuration or default configuration)
  virtual SandboxScenePackage createScenePackage(const core::PropertySet* config) = 0;
  
  virtual unique_ptr<core::PropertySet> defaultConfig() const { return nullptr; }
};

inline auto& scenesRegistry() {
  static core::ImplementationsSet<SandboxFactory> registry;
  return registry;
}
