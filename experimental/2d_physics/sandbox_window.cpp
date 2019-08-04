
#include "sandbox_window.h"

#include <QRectF>

SandboxWindow::SandboxWindow(SandboxFactory* factory) : factory_(factory) {
  setupVariables();
  newScene();
  play();
}

void SandboxWindow::newScene() {
  setSceneUi(nullptr);

  scene_package_ = factory_->createScenePackage();

  const auto extents = scene_package_.scene->extents();
  const QRectF viewport(
      extents.x, extents.y + extents.height, extents.width, -extents.height);
  setWorld(scene_package_.scene->box2dWorld(), viewport);

  setSceneUi(scene_package_.scene_ui.get());
}

void SandboxWindow::singleStep() {
  CHECK(scene_package_.scene);
  scene_package_.scene->step();
  ++step_;
  updateUI();
}

void SandboxWindow::updateUI() {
  variables_.state->setValue(stateDescription());
  variables_.step->setValue(step_);
  variables_.timestamp->setValue(scene_package_.scene->timestamp());
  variables_.objects_count->setValue(scene_package_.scene->objectsCount());
  update();
}

void SandboxWindow::setupVariables() {
  auto simulation_state_section = variablesWidget()->addSection("Simulation");
  variables_.state = simulation_state_section->addProperty("State");
  variables_.step = simulation_state_section->addProperty("Step");
  variables_.timestamp = simulation_state_section->addProperty("Timestamp");
  variables_.objects_count = simulation_state_section->addProperty("Objects count");
}
