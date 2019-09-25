
#include "touch_sensor.h"

#include <core/utils.h>

namespace phys {

TouchSensor::TouchSensor(b2Body* body, int resolution)
    : body_(body), resolution_(resolution) {
  CHECK(resolution_ > 1);
}

}  // namespace phys
