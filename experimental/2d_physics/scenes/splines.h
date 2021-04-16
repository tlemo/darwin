
#pragma once

#include "sandbox_factory.h"

#include <core/utils.h>
#include <core/sim/scene.h>
#include <core/properties.h>
#include <core/math_2d.h>
#include <core/outline_2d.h>

#include <QKeyEvent>
#include <QPixmap>

#include <memory>
#include <unordered_map>
#include <vector>
using namespace std;

namespace splines_scene {

struct Config : public core::PropertySet {
  PROPERTY(width, float, 40.0f, "Sandbox area width");
  PROPERTY(height, float, 20.0f, "Sandbox area height");
  PROPERTY(track_complexity, int, 10, "The approximate number of turns");
  PROPERTY(track_resolution, int, 500, "Number of track segments");
  PROPERTY(curb_width, float, 0.1f, "The width of the curb");
  PROPERTY(track_width, float, 1.8f, "Track width");
};

struct SceneVariables : public core::PropertySet {
  PROPERTY(objects_count, int, 0, "The total number of physics objects (bodies)");
};

class Scene : public sim::Scene {
 public:
  explicit Scene(const core::PropertySet* config);

  const SceneVariables* variables() const override { return &variables_; }

  const Config* config() const override { return &config_; }

  void preStep() override;
  void postStep(float dt) override;

  void createCurb(const math::Outline& outline, float curb_width);

 private:
  void createLight(b2Body* body, const b2Vec2& pos, const b2Color& color);
  void updateVariables();

 private:
  SceneVariables variables_;
  Config config_;
};

class SceneUi : public physics_ui::Box2dSceneUi {
 public:
  explicit SceneUi(Scene* scene);

  bool keyPressed(int key) const;

  void render(QPainter& painter, const QRectF&, bool) override;

  void step() override;

  void mousePressEvent(const QPointF& pos, QMouseEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;
  void keyReleaseEvent(QKeyEvent* event) override { key_state_[event->key()] = false; }
  void focusOutEvent() override { key_state_.clear(); }

  QString help() const override;

 private:
  void generateControlPoints();
  void updateSplines();

  void renderOutline(QPainter& painter,
                     const QPen& pen,
                     const math::Outline& outline) const;
  void renderSegments(QPainter& painter,
                      const math::Outline& outline,
                      float track_width) const;
  void renderControlPoints(QPainter& painter,
                           const QColor& color,
                           const math::Polygon& control_points) const;

 private:
  math::Polygon control_points_;
  math::Polygon outer_control_points_;
  math::Outline inner_outline_;
  math::Outline outer_outline_;

  Scene* scene_ = nullptr;
  unordered_map<int, bool> key_state_;

  bool create_curbs_ = true;
  bool render_segments_ = false;
  bool use_equidistant_outlines_ = true;
  bool render_outer_control_points_ = false;
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

}  // namespace splines_scene
