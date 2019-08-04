
#pragma once

#include "physics.h"

class Scene6 : public phys::Scene {
 public:
  Scene6();
  
  string name() const override { return "Scene6"; }
};
