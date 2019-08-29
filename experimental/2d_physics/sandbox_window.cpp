
#include "sandbox_window.h"
#include "new_scene_dialog.h"

#include <QRectF>

SandboxWindow::SandboxWindow(SandboxFactory* factory) : factory_(factory) {
  config_ = factory_->defaultConfig();
  newScene();
  play();
}

void SandboxWindow::newScene() {
  if (config_) {
    NewSceneDialog dlg(this, config_.get());
    if (dlg.exec() != QDialog::Accepted) {
      return;
    }
  }

  setSceneUi(nullptr);

  scene_package_ = factory_->createScenePackage(config_.get());
  setupVariables();

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

  if (auto scene_config = scene_package_.scene->config()) {
    for (const auto& var : scene_config->properties()) {
      auto property_item = scene_config_map_.at(var->name());
      property_item->setValue(var->value());
    }
  }

  if (auto scene_variables = scene_package_.scene->variables()) {
    for (const auto& var : scene_variables->properties()) {
      auto property_item = scene_variables_map_.at(var->name());
      property_item->setValue(var->value());
    }
  }

  update();
}

void SandboxWindow::setupVariables() {
  scene_variables_map_.clear();
  scene_config_map_.clear();
  variablesWidget()->clear();

  auto simulation_state_section = variablesWidget()->addSection("Simulation");
  variables_.state = simulation_state_section->addProperty("State");
  variables_.step = simulation_state_section->addProperty("Step");
  variables_.timestamp = simulation_state_section->addProperty("Timestamp");
  variables_.objects_count = simulation_state_section->addProperty("Objects count");

  if (auto scene_config = scene_package_.scene->config()) {
    auto scene_config_section = variablesWidget()->addSection("Config");
    for (const auto& var : scene_config->properties()) {
      scene_config_map_[var->name()] = scene_config_section->addProperty(var->name());
    }
  }

  if (auto scene_variables = scene_package_.scene->variables()) {
    auto scene_variables_section = variablesWidget()->addSection("Scene");
    for (const auto& var : scene_variables->properties()) {
      scene_variables_map_[var->name()] =
          scene_variables_section->addProperty(var->name());
    }
  }
}
