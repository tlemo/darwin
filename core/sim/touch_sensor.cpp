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
#include <utility>

namespace sim {

TouchSensor::TouchSensor(b2Body* body, int resolution)
    : body_(body), resolution_(resolution) {
  CHECK(resolution_ > 0);
}

vector<float> TouchSensor::receptors() const {
  vector<float> receptors(resolution_);
  for (const auto* ce = body_->GetContactList(); ce != nullptr; ce = ce->next) {
    if (ce->contact->IsTouching()) {
      if (resolution_ == 1) {
        // for resolution == 1 any touch would activate the single
        // receptor, so we're done
        receptors[0] = kTouchSignal;
        break;
      } else {
        b2WorldManifold manifold = {};
        ce->contact->GetWorldManifold(&manifold);
        const int point_count = ce->contact->GetManifold()->pointCount;
        if (point_count == 1) {
          const double div = pointToSensor(body_->GetLocalPoint(manifold.points[0]));
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
        } else {
          CHECK(point_count == 2);
          const double div_a = pointToSensor(body_->GetLocalPoint(manifold.points[0]));
          const double div_b = pointToSensor(body_->GetLocalPoint(manifold.points[1]));
          int sensor_a_index = int(div_a);
          int sensor_b_index = int(div_b);
          CHECK(sensor_a_index >= 0 && sensor_a_index < resolution_);
          CHECK(sensor_b_index >= 0 && sensor_b_index < resolution_);
          if (sensor_b_index < sensor_a_index) {
            std::swap(sensor_a_index, sensor_b_index);
          }
          int first = sensor_a_index;
          int last = sensor_b_index;
          const int dist_a = sensor_b_index - sensor_a_index;
          const int dist_b = sensor_a_index + resolution_ - sensor_b_index;
          if (dist_b < dist_a) {
            first = sensor_b_index;
            last = sensor_a_index + resolution_;
          }
          for (int i = first; i <= last; ++i) {
            receptors[i % resolution_] = kTouchSignal;
          }
        }
      }
    }
  }
  return receptors;
}

double TouchSensor::pointToSensor(const b2Vec2& point) const {
  // calculate the sensor index (clockwise, starting from positive y axis)
  // (note that the atan2 arguments are intentionally x, y rather than y, x)
  const double angle = atan2(double(point.x), double(point.y));
  const double sensor_slice = (2 * math::kPi) / resolution_;
  return (angle >= 0 ? angle : (angle + 2 * math::kPi)) / sensor_slice;
}

}  // namespace sim
