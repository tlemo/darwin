
#pragma once

#include <core/sim/scene.h>
#include <core/sim/car.h>
#include <core/sim/track.h>
#include <core/properties.h>
#include <core_ui/sim/box2d_widget.h>

#include <QKeyEvent>
#include <QPainterPath>

#include <memory>
using namespace std;

namespace car_race_scene {

using sim::Accelerometer;
using sim::Camera;
using sim::Compass;
using sim::TouchSensor;

struct Config : public core::PropertySet {
  PROPERTY(car_length, float, 1.2f, "Car length");
  PROPERTY(max_forward_force, float, 1.0f, "Max forward move force");
  PROPERTY(max_reverse_force, float, 0.5f, "Max reverse move force");
  PROPERTY(max_steer_angle, float, 40.0f, "Max steering angle");
  PROPERTY(tire_traction, float, 1.5f, "Tire traction (max lateral impulse)");

  PROPERTY(track_width, float, 2.5f, "Track width");
  PROPERTY(track_complexity, int, 10, "The approximate number of turns");
  PROPERTY(track_resolution, int, 500, "Number of track segments");
  PROPERTY(curb_width, float, 0.1f, "Curb width");
  PROPERTY(curb_friction, float, 0.5f, "Track's curb friction");
  PROPERTY(track_gates, bool, true, "Generate track gates");
  PROPERTY(solid_gate_posts, bool, true, "Solid gate posts");
};

struct SceneVariables : public core::PropertySet {
  PROPERTY(car_x, float, 0, "Car x coordinate");
  PROPERTY(car_y, float, 0, "Car y coordinate");
  PROPERTY(car_velocity, float, 0, "Car velocity");
  PROPERTY(car_dir, float, 0, "Heading angle");
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

  const Camera* camera() const override { return car_->camera(); }
  const TouchSensor* touchSensor() const override { return car_->touchSensor(); }
  const Accelerometer* accelerometer() const override { return car_->accelerometer(); }
  const Compass* compass() const override { return car_->compass(); }

  void preStep() override;
  void postStep(float dt) override;

  sim::Car* car() { return car_.get(); }

  const sim::Track* track() const { return track_.get(); }

 private:
  unique_ptr<sim::Car> createCar();
  unique_ptr<sim::Track> createTrack();
  void updateVariables();

 private:
  unique_ptr<sim::Car> car_;
  unique_ptr<sim::Track> track_;
  int track_distance_ = 0;

  SceneVariables variables_;
  Config config_;
};

class SceneUi : public physics_ui::Box2dSceneUi {
 public:
  explicit SceneUi(Scene* scene);

  bool keyPressed(int key) const;

  void render(QPainter& painter, const QRectF& viewport) override;

  void step() override;

 protected:
  void keyPressEvent(QKeyEvent* event) override { key_state_[event->key()] = true; }
  void keyReleaseEvent(QKeyEvent* event) override { key_state_[event->key()] = false; }
  void focusOutEvent() override { key_state_.clear(); }

 private:
  void renderCamera(QPainter& painter, const sim::Camera* camera) const;
  void renderPath(QPainter& painter) const;
  void renderTrack(QPainter& painter) const;
  void renderCurrentSegment(QPainter& painter) const;

 private:
  Scene* scene_ = nullptr;
  unordered_map<int, bool> key_state_;
  QPainterPath car_path_;
};

}  // namespace car_race_scene
