
#pragma once

#include "sandbox_factory.h"

#include <core/physics/scene.h>
#include <core/physics/accelerometer.h>
#include <core/physics/camera.h>
#include <core/physics/compass.h>
#include <core/physics/touch_sensor.h>
#include <core/properties.h>

#include <QKeyEvent>
#include <QPixmap>

#include <memory>
#include <unordered_map>
#include <random>
#include <istream>
#include <ostream>
#include <iomanip>
using namespace std;

inline ostream& operator<<(ostream& stream, const b2Vec2& v) {
  using core::operator<<;
  return stream << std::setprecision(3) << "{ " << v.x << ", " << v.y << " }";
}

inline istream& operator>>(istream& stream, b2Vec2& v) {
  using core::operator>>;
  return stream >> "{" >> v.x >> "," >> v.y >> "}";
}

namespace hectic_drone_scene {

using physics::Camera;
using physics::TouchSensor;
using physics::Accelerometer;
using physics::Compass;

struct Config : public core::PropertySet {
  PROPERTY(drone_radius, float, 0.5f, "Drone size");
  PROPERTY(move_force, float, 5.0f, "The force used to move the drone");
  PROPERTY(rotate_torque, float, 1.0f, "The torque used to rotate the drone");
};

struct SceneVariables : public core::PropertySet {
  PROPERTY(drone_x, float, 0, "Drone x coordinate");
  PROPERTY(drone_y, float, 0, "Drone y coordinate");
  PROPERTY(drone_vx, float, 0, "Drone velocity (x component)");
  PROPERTY(drone_vy, float, 0, "Drone velocity (y component)");
  PROPERTY(drone_dir, float, 0, "Heading angle");
  PROPERTY(start_pos, b2Vec2, b2Vec2(0, 0), "Starting position for current segment");
  PROPERTY(target_pos, b2Vec2, b2Vec2(0, 0), "Target position for current segment");
};

class Scene : public physics::Scene {
 public:
  explicit Scene(const core::PropertySet* config);

  const SceneVariables* variables() const override { return &variables_; }

  const Config* config() const override { return &config_; }

  const Camera* camera() const override { return camera_.get(); }
  const TouchSensor* touchSensor() const override { return touch_sensor_.get(); }
  const Accelerometer* accelerometer() const override { return accelerometer_.get(); }
  const Compass* compass() const override { return compass_.get(); }

  void preStep() override;
  void postStep(float dt) override;

  void moveDrone(const b2Vec2& force);
  void rotateDrone(float torque);

  void addBalloon(float x, float y, float radius);
  void addBox(float x, float y, float sx, float sy);

 private:
  b2Body* createDrone(const b2Vec2& pos, float radius);
  void createLight(b2Body* body, const b2Vec2& pos, const b2Color& color);
  void generateTargetPos();
  void updateVariables();

 private:
  b2Body* drone_ = nullptr;
  b2Vec2 start_pos_;
  b2Vec2 target_pos_;

  default_random_engine rnd_{ random_device{}() };

  unique_ptr<Camera> camera_;
  unique_ptr<TouchSensor> touch_sensor_;
  unique_ptr<Accelerometer> accelerometer_;
  unique_ptr<Compass> compass_;
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
  void renderCamera(QPainter& painter, const physics::Camera* camera) const;
  void renderDrone(QPainter& painter) const;
  void renderTarget(QPainter& painter) const;

 private:
  Scene* scene_ = nullptr;
  unordered_map<int, bool> key_state_;
  QPixmap drone_pixmap_{ ":/resources/drone.png" };
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

}  // namespace hectic_drone_scene
