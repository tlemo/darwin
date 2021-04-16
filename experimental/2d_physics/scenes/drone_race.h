
#pragma once

#include <core/sim/scene.h>
#include <core/sim/drone.h>
#include <core/sim/track.h>
#include <core/properties.h>
#include <core_ui/sim/box2d_widget.h>

#include <QKeyEvent>
#include <QPainterPath>

#include <memory>
using namespace std;

namespace drone_race_scene {

using sim::Accelerometer;
using sim::Camera;
using sim::Compass;
using sim::TouchSensor;

struct Config : public core::PropertySet {
  PROPERTY(drone_radius, float, 0.3f, "Drone size");
  PROPERTY(move_force, float, 10.0f, "The force used to move the drone");
  PROPERTY(lateral_force, float, 5.0f, "The lateral force used to move the drone");
  PROPERTY(rotate_torque, float, 0.5f, "The torque used to rotate the drone");

  PROPERTY(track_width, float, 2.0f, "Track width");
  PROPERTY(track_complexity, int, 20, "The approximate number of turns");
  PROPERTY(track_resolution, int, 500, "Number of track segments");
  PROPERTY(curb_width, float, 0.1f, "Curb width");
  PROPERTY(curb_friction, float, 0.5f, "Track's curb friction");
  PROPERTY(track_gates, bool, true, "Generate track gates");
  PROPERTY(solid_gate_posts, bool, true, "Solid gate posts");
};

struct SceneVariables : public core::PropertySet {
  PROPERTY(drone_x, float, 0, "Drone x coordinate");
  PROPERTY(drone_y, float, 0, "Drone y coordinate");
  PROPERTY(drone_vx, float, 0, "Drone velocity (x component)");
  PROPERTY(drone_vy, float, 0, "Drone velocity (y component)");
  PROPERTY(drone_dir, float, 0, "Heading angle");
  PROPERTY(track_distance, int, 0, "Current track segment");
};

class Scene : public sim::Scene {
 public:
  static constexpr float kWidth = 40;
  static constexpr float kHeight = 20;

 public:
  explicit Scene(const core::PropertySet* config);

  const SceneVariables* variables() const override { return &variables_; }

  const Config* config() const override { return &config_; }

  const Camera* camera() const override { return drone_->camera(); }
  const TouchSensor* touchSensor() const override { return drone_->touchSensor(); }
  const Accelerometer* accelerometer() const override { return drone_->accelerometer(); }
  const Compass* compass() const override { return drone_->compass(); }

  void postStep(float dt) override;

  void moveDrone(const b2Vec2& force);
  void rotateDrone(float torque);

  sim::Drone* drone() { return drone_.get(); }

  const sim::Track* track() const { return track_.get(); }

 private:
  unique_ptr<sim::Drone> createDrone();
  unique_ptr<sim::Track> createTrack();
  void updateVariables();

 private:
  unique_ptr<sim::Drone> drone_;
  unique_ptr<sim::Track> track_;
  int track_distance_ = 0;

  SceneVariables variables_;
  Config config_;
};

class SceneUi : public physics_ui::Box2dSceneUi {
 public:
  explicit SceneUi(Scene* scene);

  bool keyPressed(int key) const;

  void render(QPainter& painter, const QRectF& viewport, bool debug) override;

  void step() override;

 protected:
  void keyPressEvent(QKeyEvent* event) override { key_state_[event->key()] = true; }
  void keyReleaseEvent(QKeyEvent* event) override { key_state_[event->key()] = false; }
  void focusOutEvent() override { key_state_.clear(); }

 private:
  void renderCamera(QPainter& painter, const sim::Camera* camera) const;
  void renderDrone(QPainter& painter, const sim::Drone* drone) const;
  void renderPath(QPainter& painter) const;
  void renderTrack(QPainter& painter) const;
  void renderCurrentSegment(QPainter& painter) const;

 private:
  Scene* scene_ = nullptr;
  unordered_map<int, bool> key_state_;
  QPainterPath drone_path_;
  const QPixmap drone_pixmap_{ ":/resources/drone.png" };
};

}  // namespace drone_race_scene
