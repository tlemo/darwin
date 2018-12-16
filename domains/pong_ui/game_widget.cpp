// Copyright 2018 The Darwin Neuroevolution Framework Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "game_widget.h"
#include "human_player.h"
#include "new_sandbox_dialog.h"
#include "ui_game_widget.h"

#include <domains/pong/ann_player.h>
#include <domains/pong/player.h>
#include <domains/pong/test_players.h>

#include <core/darwin.h>
#include <core/evolution.h>
#include <core/logging.h>

#include <memory>
#include <limits>
using namespace std;

namespace pong_ui {

GameWidget::GameWidget() : ui(new Ui::GameWidget), game_(numeric_limits<int>::max()) {
  ui->setupUi(this);
  ui->pong_widget->setDebugRendering(ui->debug_rendering->isChecked());

  setupVariables();

  connect(ui->pong_widget, &PongWidget::sigUpdate, this, &GameWidget::updateUI);
}

GameWidget::~GameWidget() {
  ui->pong_widget->reset();
  game_.reset();
  left_player_.reset();
  right_player_.reset();

  delete ui;
}

unique_ptr<pong::Player> GameWidget::newPlayer(const QString& type,
                                               int generation) const {
  if (type == NewSandboxDialog::kAiPlayer) {
    auto snapshot = darwin::evolution()->snapshot();
    auto summary = snapshot.trace->generationSummary(generation);
    auto player = make_unique<pong::AnnPlayer>();
    player->generation = generation;
    player->grow(summary.champion.get());
    return player;
  } else if (type == NewSandboxDialog::kHumanPlayer) {
    return make_unique<pong_ui::HumanPlayer>(ui->pong_widget);
  } else if (type == NewSandboxDialog::kHandcraftedPlayer) {
    return make_unique<pong::HandcraftedPlayer>();
  } else {
    FATAL("Unexpected player type");
  }
}

bool GameWidget::setup() {
  NewSandboxDialog dlg;
  if (dlg.exec() != QDialog::Accepted)
    return false;

  try {
    left_player_ = newPlayer(dlg.leftPlayerType(), dlg.leftPlayerGeneration());
    right_player_ = newPlayer(dlg.rightPlayerType(), dlg.rightPlayerGeneration());
  } catch (const exception& e) {
    core::log("Can't create the new sandbox players: %s\n", e.what());
    return false;
  }

  game_.newGame(left_player_.get(), right_player_.get());

  ui->pong_widget->setGame(&game_);
  ui->pong_widget->setFocus();

  setWindowTitle(QString("%1 vs %2")
                     .arg(QString::fromStdString(left_player_->name()))
                     .arg(QString::fromStdString(right_player_->name())));

  return true;
}

void GameWidget::setupVariables() {
  auto config_section = ui->variables->addSection("Configuration");
  variables_.left_player = config_section->addProperty("Left player");
  variables_.right_player = config_section->addProperty("Right player");

  auto game_state_section = ui->variables->addSection("Game state");
  variables_.current_set = game_state_section->addProperty("Current set");
  variables_.game_step = game_state_section->addProperty("Game step");
  variables_.score_left = game_state_section->addProperty("Score left player");
  variables_.score_right = game_state_section->addProperty("Score right player");
}

void GameWidget::updateUI() {
  variables_.left_player->setValue(left_player_->name());
  variables_.right_player->setValue(right_player_->name());

  variables_.current_set->setValue(game_.currentSet());
  variables_.game_step->setValue(game_.currentStep());
  variables_.score_left->setValue(game_.scoreP1());
  variables_.score_right->setValue(game_.scoreP2());
}

void GameWidget::on_debug_rendering_toggled(bool checked) {
  ui->pong_widget->setDebugRendering(checked);
}

}  // namespace pong_ui
