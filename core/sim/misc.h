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

#pragma once

#include <third_party/box2d/box2d.h>

namespace sim {

// TODO: refactor these helpers
b2Body* addBall(float x, float y, float r, b2World* world);
b2Body* addBox(float x, float y, float w, float h, b2World* world);
b2Body* addCross(float x, float y, float w, float h, b2World* world);
b2Body* addBullet(float x, float y, float dx, float dy, b2World* world);
b2Body* addBoxProjectile(float x, float y, float dx, float dy, b2World* world);

}  // namespace sim
