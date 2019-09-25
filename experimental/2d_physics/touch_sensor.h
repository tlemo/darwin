
#pragma once

#include <third_party/box2d/box2d.h>

#include <vector>
using namespace std;

namespace phys {

class TouchSensor {
 public:
  TouchSensor(b2Body* body, int resolution);

  b2Body* body() const { return body_; }

  int resolution() const { return resolution_; }

 private:
  b2Body* body_ = nullptr;
  const int resolution_ = 0;
};

}  // namespace phys
