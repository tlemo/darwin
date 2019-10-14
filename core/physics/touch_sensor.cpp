// Copyright The Darwin Neuroevolution Framework Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "touch_sensor.h"

#include <core/math_2d.h>
#include <core/utils.h>

#include <math.h>

namespace physics {

TouchSensor::TouchSensor(b2Body* body, int resolution)
    : body_(body), resolution_(resolution) {
  CHECK(resolution_ > 0);
}

vector<float> TouchSensor::receptors() const {
  vector<float> receptors(resolution_);
  for (const auto* ce = body_->GetContactList(); ce != nullptr; ce = ce->next) {
    if (ce->contact->IsTouching()) {
      // TODO: add support for 2-point contact manifolds
      CHECK(ce->contact->GetManifold()->pointCount == 1);
      if (resolution_ == 1) {
        // for resolution == 1 any touch would activate the single
        // receptor, so we're done
        receptors[0] = kTouchSignal;
        break;
      } else {
        b2WorldManifold manifold = {};
        ce->contact->GetWorldManifold(&manifold);
        const b2Vec2 point = body_->GetLocalPoint(manifold.points[0]);

        // calculate the sensor index (clockwise, starting from positive y axis)
        // (note that the atan2 arguments are intentionally x, y ranther than y, x)
        const double angle = atan2(double(point.x), double(point.y));
        const double sensor_slice = (2 * math::kPi) / resolution_;
        const double div = (angle >= 0 ? angle : (angle + 2 * math::kPi)) / sensor_slice;
        const int sensor_index = int(div);
        CHECK(sensor_index >= 0 && sensor_index < resolution_);
        receptors[sensor_index] = kTouchSignal;
        
        // contacts near the boundary between receptors will trigger the adjacent
        // receptor as well (consider resolution == 3 and a contact right in front,
        // we don't want to activate just the receptor 0 since it would misleadingly 
        // suggest the contact is to the right rather than in front)
        const double rem = div - floor(div);
        if (rem < kReceptorBoundaryTolerance) {
          receptors[(sensor_index + resolution_ - 1) % resolution_] = kTouchSignal;
        } else if (rem > 1 - kReceptorBoundaryTolerance) {
          receptors[(sensor_index + 1) % resolution_] = kTouchSignal;
        }
      }
    }
  }
  return receptors;
}

}  // namespace physics
