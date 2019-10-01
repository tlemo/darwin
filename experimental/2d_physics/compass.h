
#pragma once

#include <third_party/box2d/box2d.h>

namespace phys {

class Compass {
 public:
  Compass(b2Body* body);

  b2Body* body() const { return body_; }

  const b2Vec2 heading() const;

 private:
  b2Body* body_ = nullptr;
};

}  // namespace phys
