
#pragma once

#include "sandbox_factory.h"

#include <core/utils.h>
#include <core/sim/scene.h>
#include <core/sim/accelerometer.h>
#include <core/sim/camera.h>
#include <core/sim/compass.h>
#include <core/sim/touch_sensor.h>
#include <core/properties.h>

#include <QKeyEvent>
#include <QPixmap>

#include <memory>
#include <unordered_map>
using namespace std;

namespace car_scene {

using sim::Accelerometer;
using sim::Camera;
using sim::Compass;
using sim::TouchSensor;

struct CarConfig {
  b2Vec2 position{};
  float angle = 0;
  float length = 0;
  bool camera = false;
  float camera_fov = 90;
  int camera_resolution = 64;
  bool camera_depth = false;
  bool touch_sensor = false;
  int touch_resolution = 16;
  bool accelerometer = false;
  bool compass = false;
  float max_forward_force = 15.0f;
  float max_reverse_force = 4.0f;
  float max_steer_angle = 40.0f;
  float density = 0.01f;
  float tire_traction = 0.1f;
  b2Color color;
};

class Car : public core::NonCopyable {
 public:
  Car(b2World* world, const CarConfig& config);

  void preStep();
  void postStep(float dt);

  //! Accelerates the car on the forward/reverse direction
  //! (positive == forward, negative == reverse)
  void accelerate(float force);

  //! Adjust the steering. `steer_wheeel_position` is the desired
  //! steering wheel position, -1 = max left turn angle, +1 = max right turn angle.
  void steer(float steer_wheel_position);

  //! Apply brakes with the given intensity (0 = no brakes, 1 = maximum braking)
  void brake(float intensity);

  // sensors
  const Camera* camera() const { return camera_.get(); }
  const sim::TouchSensor* touchSensor() const { return touch_sensor_.get(); }
  const sim::Accelerometer* accelerometer() const { return accelerometer_.get(); }
  const sim::Compass* compass() const { return compass_.get(); }

  b2Body* body() { return car_body_; }
  const b2Body* body() const { return car_body_; }

  const auto& config() const { return config_; }

 private:
  void createSensors();
  void createWheelFixture(b2Body* body, const b2Vec2& pos);
  void createLightFixture(const b2Vec2& pos, const b2Color& color);
  b2RevoluteJoint* createTurningWheel(b2World* world, const b2Vec2& pos);

  void updateSteering();
  void applyBrakeImpulse(float intensity,
                         const b2Vec2& wheel_normal,
                         const b2Vec2& wheel_center);
  void applyTireLateralImpulse(const b2Vec2& wheel_normal, const b2Vec2& wheel_center);

  float width() const { return config_.length * 0.5f; }
  float frontAxleOffset() const { return config_.length * 0.3f; }
  float rearAxleOffset() const { return config_.length * -0.3f; }

 private:
  b2Body* car_body_ = nullptr;
  b2RevoluteJoint* left_wheel_joint_ = nullptr;
  b2RevoluteJoint* right_wheel_joint_ = nullptr;
  float target_steer_ = 0;
  float brake_pedal_ = 0;
  unique_ptr<Camera> camera_;
  unique_ptr<TouchSensor> touch_sensor_;
  unique_ptr<Accelerometer> accelerometer_;
  unique_ptr<Compass> compass_;
  const CarConfig config_;
};

struct Config : public core::PropertySet {
  PROPERTY(car_length, float, 3.0f, "Car length");
  PROPERTY(max_forward_force, float, 15.0f, "Max forward move force");
  PROPERTY(max_reverse_force, float, 4.0f, "Max reverse move force");
  PROPERTY(max_steer_angle, float, 40.0f, "Max steering angle");
  PROPERTY(tire_traction, float, 0.1f, "Tire traction (max lateral impulse)");
};

struct SceneVariables : public core::PropertySet {
  PROPERTY(car_x, float, 0, "Car x coordinate");
  PROPERTY(car_y, float, 0, "Car y coordinate");
  PROPERTY(car_velocity, float, 0, "Car velocity");
  PROPERTY(car_dir, float, 0, "Heading angle");
};

class Scene : public sim::Scene {
  static constexpr float kWidth = 40;
  static constexpr float kHeight = 20;

 public:
  explicit Scene(const core::PropertySet* config);

  const SceneVariables* variables() const override { return &variables_; }

  const Config* config() const override { return &config_; }

  const Camera* camera() const override { return car_->camera(); }
  const TouchSensor* touchSensor() const override { return car_->touchSensor(); }
  const Accelerometer* accelerometer() const override { return car_->accelerometer(); }
  const Compass* compass() const override { return car_->compass(); }

  Car* car() { return car_.get(); }

  void preStep() override;
  void postStep(float dt) override;

  void addBalloon(float x, float y, float radius);
  void addBox(float x, float y, float sx, float sy);

 private:
  void createLight(b2Body* body, const b2Vec2& pos, const b2Color& color);
  void updateVariables();

 private:
  unique_ptr<Car> car_;
  unique_ptr<Car> dummy_car_;
  SceneVariables variables_;
  Config config_;
};

class SceneUi : public physics_ui::Box2dSceneUi {
 public:
  explicit SceneUi(Scene* scene) : scene_(scene) {}

  bool keyPressed(int key) const {
    const auto it = key_state_.find(key);
    return it != key_state_.end() ? it->second : false;
  }

  void render(QPainter& painter, const QRectF&) override;

  void step() override;

  void mousePressEvent(const QPointF& pos, QMouseEvent* event) override;

  void keyPressEvent(QKeyEvent* event) override { key_state_[event->key()] = true; }

  void keyReleaseEvent(QKeyEvent* event) override { key_state_[event->key()] = false; }

  void focusOutEvent() override { key_state_.clear(); }

 private:
  void renderCamera(QPainter& painter, const sim::Camera* camera) const;

 private:
  Scene* scene_ = nullptr;
  unordered_map<int, bool> key_state_;
};

class Factory : public SandboxFactory {
  SandboxScenePackage createScenePackage(const core::PropertySet* config) override {
    auto scene = make_unique<Scene>(config);
    auto scene_ui = make_unique<SceneUi>(scene.get());
    return { std::move(scene), std::move(scene_ui) };
  }

  unique_ptr<core::PropertySet> defaultConfig() const override {
    return make_unique<Config>();
  }
};

}  // namespace car_scene
