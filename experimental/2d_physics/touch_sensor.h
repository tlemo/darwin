
#pragma once

#include <third_party/box2d/box2d.h>

#include <vector>
using namespace std;

namespace phys {

//! A basic touch sensor attached to a physical body
//! 
//! It maps collisions to sensors spaced evenly in slices around the body's coordinate
//! origin (sensor 0 starts at the positive y axis, advancing clockwise)
//! 
class TouchSensor {
  static constexpr float kTouchSignal = 1.0f;
  static constexpr double kReceptorBoundaryTolerance = 0.1;
  
 public:
  TouchSensor(b2Body* body, int resolution);

  b2Body* body() const { return body_; }

  int resolution() const { return resolution_; }

  vector<float> receptors() const;  

 private:
  b2Body* body_ = nullptr;
  const int resolution_ = 0;
};

}  // namespace phys
