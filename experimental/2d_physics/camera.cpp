
#include "camera.h"

#include <limits>

namespace phys {

constexpr float kInfinity = std::numeric_limits<float>::infinity();
constexpr float kEpsilon = std::numeric_limits<float>::epsilon();

struct RayCastCallback : public b2RayCastCallback {
  b2Vec2 point;
  b2Vec2 normal;
  float fraction = 0;
  b2Fixture* fixture = nullptr;
  const float min_fraction = 0;

  explicit RayCastCallback(float min_fraction) : min_fraction(min_fraction) {}

  float32 ReportFixture(b2Fixture* fixture,
                        const b2Vec2& point,
                        const b2Vec2& normal,
                        float32 fraction) override {
    assert(fraction >= 0);
    assert(fraction <= 1);
    if (fraction >= min_fraction) {
      this->point = point;
      this->normal = normal;
      this->fixture = fixture;
      this->fraction = fraction;
      return fraction;
    } else {
      return -1;
    }
  }
};

struct ShadowRayCastCallback : public b2RayCastCallback {
  b2Fixture* const self = nullptr;
  bool intersection = false;

  explicit ShadowRayCastCallback(b2Fixture* self) : self(self) {}

  float32 ReportFixture(b2Fixture* fixture,
                        const b2Vec2& /*point*/,
                        const b2Vec2& /*normal*/,
                        float32 /*fraction*/) override {
    if (fixture == self) {
      return -1;
    } else {
      intersection = true;
      return 0;
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

Receptor Camera::castRay(const b2Vec2& ray_start,
                         const b2Vec2& ray_end,
                         float min_fraction,
                         int depth) const {
  const auto world = body_->GetWorld();
  RayCastCallback raycast(min_fraction);
  world->RayCast(&raycast, ray_start, ray_end);

  // if no intersection, return the background color
  if (raycast.fixture == nullptr) {
    return Receptor(b2Color(0, 0, 0), 1.0f);
  }

  b2Color color;  // TODO: ambient color
  b2Color specular_color;

  const b2Body* body = raycast.fixture->GetBody();
  const b2Material& material = raycast.fixture->GetMaterial();
  const b2Vec2 local_point = body->GetLocalPoint(raycast.point);
  const b2Vec2 local_normal = body->GetLocalVector(raycast.normal);
  const b2Vec2 V = body->GetLocalVector(ray_start - ray_end).Normalized();

  // emissive lighting
  assert(material.emit_intensity >= 0 && material.emit_intensity <= 1);
  if (material.emit_intensity > 0) {
    color = color + material.color * material.emit_intensity;
  }

  // basic illumination (diffuse, specular), including shadows
  for (auto light = world->GetLightList(); light != nullptr; light = light->GetNext()) {
    const auto& ldef = light->GetDef();
    const auto global_light_pos = ldef.body->GetWorldPoint(ldef.position);
    const auto local_light_pos = body->GetLocalPoint(global_light_pos);

    // shadows?
    if (render_shadows_) {
      ShadowRayCastCallback shadow_raycast(raycast.fixture);
      world->RayCast(&shadow_raycast, raycast.point, global_light_pos);
      if (shadow_raycast.intersection) {
        continue;
      }
    }

    // diffuse lighting
    const b2Vec2 L = (local_light_pos - local_point).Normalized();
    const float diffuse_intensity = ldef.intensity * fmaxf(b2Dot(local_normal, L), 0);
    color = color + ldef.color * diffuse_intensity;

    // specular lighing?
    assert(material.shininess >= 0);
    if (render_specular_ && material.shininess > 0) {
      const b2Vec2 H = (L + V) * 0.5f;
      const float s = powf(fmaxf(b2Dot(local_normal, H), 0), material.shininess);
      const float specular_intensity = ldef.intensity * s;
      specular_color = specular_color + ldef.color * specular_intensity;
    }
  }

  // final color modulation
  color = color * material.color + specular_color;

  // saturation
  color.r = fminf(color.r, 1.0f);
  color.g = fminf(color.g, 1.0f);
  color.b = fminf(color.b, 1.0f);

  assert(raycast.fraction > 0 && raycast.fraction <= 1);
  return Receptor(color, raycast.fraction);
}

vector<Receptor> Camera::render() const {
  vector<Receptor> image(resolution_);
  const float dx = width_ / resolution_;
  const float far_near_ratio = far_ / near_;
  const float near_far_ratio = near_ / far_;
  float near_x = -(width_ - dx) / 2;
  const b2Vec2 ray_start = body_->GetWorldPoint(b2Vec2(0, 0));
  for (int i = 0; i < resolution_; ++i) {
    const float far_x = near_x * far_near_ratio;
    const b2Vec2 ray_end = body_->GetWorldPoint(b2Vec2(far_x, far_));
    image[i] = castRay(ray_start, ray_end, near_far_ratio);
    near_x += dx;
  }
  return image;
}

}  // namespace phys
