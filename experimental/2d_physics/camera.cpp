
#include "camera.h"

namespace phys {

struct RayCastCallback : public b2RayCastCallback {
  b2Vec2 point;
  b2Vec2 normal;
  b2Fixture* fixture = nullptr;
  const float min_fraction = 0;

  RayCastCallback(float min_fraction) : min_fraction(min_fraction) {}

  float32 ReportFixture(b2Fixture* fixture,
                        const b2Vec2& point,
                        const b2Vec2& normal,
                        float32 fraction) override {
    if (fraction >= min_fraction) {
      this->point = point;
      this->normal = normal;
      this->fixture = fixture;
      return fraction;
    } else {
      return -1;
    }
  }
};

Camera::Camera(b2Body* body, float width, float near, float far, int resolution)
    : body_(body), width_(width), near_(near), far_(far), resolution_(resolution) {
  CHECK(resolution_ > 0);
  CHECK(width_ > 0);
  CHECK(near_ > 0);
  CHECK(far_ > near_);
}

vector<Color> Camera::render() const {
  vector<Color> image(resolution_);
  const float dx = width_ / resolution_;
  const float far_near_ratio = far_ / near_;
  const float near_far_ratio = near_ / far_;
  const auto world = body_->GetWorld();
  float near_x = -(width_ - dx) / 2;
  const b2Vec2 ray_start = body_->GetWorldPoint(b2Vec2(0, 0));
  for (int i = 0; i < resolution_; ++i) {
    const float far_x = near_x * far_near_ratio;
    const b2Vec2 ray_end = body_->GetWorldPoint(b2Vec2(far_x, far_));
    RayCastCallback raycast(near_far_ratio);
    world->RayCast(&raycast, ray_start, ray_end);
    if (raycast.fixture != nullptr) {
      // TODO: fixture color
      image[i] = Color(1, 1, 1);
    } else {
      // TODO: background color
      image[i] = Color(0, 0, 0);
    }
    near_x += dx;
  }
  return image;
}

}  // namespace phys
