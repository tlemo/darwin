
#pragma once

#include <core/utils.h>
#include <third_party/box2d/box2d.h>

#include <vector>
using namespace std;

namespace phys {

struct Receptor {
  float r = 0;
  float g = 0;
  float b = 0;
  float distance = 0;

  Receptor() = default;

  explicit Receptor(float r, float g, float b, float distance)
      : r(r), g(g), b(b), distance(distance) {}
};

class Camera {
 public:
  Camera(b2Body* body, float width, float near, float far, int resolution);

  vector<Receptor> render() const;

  b2Body* body() const { return body_; }

  float width() const { return width_; }
  float near() const { return near_; }
  float far() const { return far_; }
  int resolution() const { return resolution_; }

 private:
  b2Body* body_ = nullptr;
  const float width_ = 0;
  const float near_ = 0;
  const float far_ = 0;
  const int resolution_ = 0;
};

}  // namespace phys
