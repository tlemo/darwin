
#pragma once

#include "camera.h"
#include "physics.h"
#include "sandbox_factory.h"

#include <core/properties.h>
#include <core_ui/box2d_widget.h>

#include <QKeyEvent>
#include <QPixmap>

#include <memory>
#include <unordered_map>
using namespace std;

namespace drone_scene {

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
};

class Scene : public phys::Scene {
 public:
  explicit Scene(const core::PropertySet* config);

  const SceneVariables* variables() const override { return &variables_; }

  const Config* config() const override { return &config_; }

  const phys::Camera* camera() const { return camera_.get(); }

  void postStep() override { updateVariables(); }

  void moveDrone(const b2Vec2& force);
  void rotateDrone(float torque);

  void addBalloon(float x, float y, float radius);
  void addBox(float x, float y, float sx, float sy);

 private:
  void updateVariables();

 private:
  b2Body* drone_ = nullptr;
  unique_ptr<phys::Camera> camera_;
  SceneVariables variables_;
  Config config_;
};

class SceneUi : public core_ui::Box2dSceneUi {
  static constexpr float kCartImpulse = 1.0f;

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
  void renderCamera(QPainter& painter, const phys::Camera* camera) const;
  void renderDrone(QPainter& painter) const;

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

}  // namespace drone_scene
