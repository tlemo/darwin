
#include "compass.h"

#include <core/utils.h>

namespace phys {

Compass::Compass(b2Body* body) : body_(body) {}

const b2Vec2 Compass::heading() const {
  return body_->GetLocalVector(b2Vec2(0, 1));
}

}  // namespace phys
