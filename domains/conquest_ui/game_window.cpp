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

#include "game_window.h"
#include "new_sandbox_dialog.h"
#include "ui_game_window.h"

#include <domains/conquest/ann_player.h>
#include <domains/conquest/player.h>
#include <domains/conquest/test_players.h>

#include <core/darwin.h>
#include <core/evolution.h>
#include <core/logging.h>

#include <QString>

namespace conquest_ui {

GameWindow::GameWindow() : ui(new Ui::GameWindow) {
  ui->setupUi(this);
  ui->board_widget->setDebugRendering(ui->debug_rendering->isChecked());

  setupVariables();

  connect(ui->board_widget, &BoardWidget::sigPause, this, &GameWindow::pauseGame);
  connect(ui->board_widget, &BoardWidget::sigResume, this, &GameWindow::resumeGame);

  updateUI();
}

GameWindow::~GameWindow() {
  pauseGame();
  ui->board_widget->reset();
  game_.reset();
  blue_player_.reset();
  red_player_.reset();

  delete ui;
}

unique_ptr<conquest::Player> GameWindow::newPlayer(const QString& type,
                                                   int generation) const {
  if (type == NewSandboxDialog::kAiPlayer) {
    auto snapshot = darwin::evolution()->snapshot();
    auto summary = snapshot.trace->generationSummary(generation);
    auto player = make_unique<conquest::AnnPlayer>();
    player->generation = generation;
    player->grow(summary.champion.get());
    return player;
  } else if (type == NewSandboxDialog::kHumanPlayer) {
    return make_unique<conquest::DummyPlayer>();
  } else if (type == NewSandboxDialog::kHandcraftedPlayer) {
    return make_unique<conquest::HandcraftedPlayer>();
  } else if (type == NewSandboxDialog::kRandomPlayer) {
    return make_unique<conquest::RandomPlayer>();
  } else {
    FATAL("Unexpected player type");
  }
}

bool GameWindow::setup() {
  NewSandboxDialog dlg;
  if (dlg.exec() != QDialog::Accepted)
    return false;

  try {
    blue_player_ = newPlayer(dlg.bluePlayerType(), dlg.bluePlayerGeneration());
    red_player_ = newPlayer(dlg.redPlayerType(), dlg.redPlayerGeneration());
  } catch (const exception& e) {
    core::log("Can't create the new sandbox players: %s\n", e.what());
    return false;
  }

  board_ = conquest::Board::getBoard(conquest::g_config.board);
  CHECK(board_ != nullptr);

  game_ = make_unique<conquest::Game>(conquest::g_config.max_steps, board_);
  game_->newGame(blue_player_.get(), red_player_.get());

  ui->board_widget->setGame(game_.get());
  ui->board_widget->setFocus();

  setWindowTitle(QString("%1 vs %2")
                     .arg(QString::fromStdString(blue_player_->name()))
                     .arg(QString::fromStdString(red_player_->name())));

  timer_.stop();
  timer_.disconnect();
  connect(&timer_, &QTimer::timeout, this, &GameWindow::gameStep);
  timer_.start(50);  // TODO: config this

  return true;
}

void GameWindow::setupVariables() {
  auto config_section = ui->variables->addSection("Configuration");
  variables_.blue_player = config_section->addProperty("Blue player");
  variables_.red_player = config_section->addProperty("Red player");

  auto game_state_section = ui->variables->addSection("Game state");
  variables_.state = game_state_section->addProperty("State");
  variables_.blue_units = game_state_section->addProperty("Blue units");
  variables_.red_units = game_state_section->addProperty("Red units");
  variables_.blue_nodes = game_state_section->addProperty("Blue nodes");
  variables_.red_nodes = game_state_section->addProperty("Red nodes");
  variables_.game_step = game_state_section->addProperty("Game step");
}

void GameWindow::updateUI() {
  if (game_) {
    ui->game_stats->updateCharts(game_.get());

    auto stats = game_->computeStats();

    const char* game_state = "Unknown";
    switch (game_->state()) {
      case conquest::Game::State::None:
        game_state = "None";
        break;
      case conquest::Game::State::InProgress:
        game_state = "InProgress";
        break;
      case conquest::Game::State::BlueWins:
        game_state = "Blue Wins!";
        break;
      case conquest::Game::State::RedWins:
        game_state = "Red Wins!";
        break;
      case conquest::Game::State::Draw:
        game_state = "Draw";
        break;
    }

    variables_.blue_player->setValue(blue_player_->name());
    variables_.red_player->setValue(red_player_->name());

    variables_.state->setValue(game_state);
    variables_.blue_units->setValue(stats.blue_units, 0, 'f', 2);
    variables_.red_units->setValue(stats.red_units, 0, 'f', 2);
    variables_.blue_nodes->setValue(stats.blue_nodes);
    variables_.red_nodes->setValue(stats.red_nodes);
    variables_.game_step->setValue(game_->currentStep());
  } else {
    ui->game_stats->resetCharts();
  }

  update();
}

void GameWindow::gameStep() {
  game_->gameStep();
  updateUI();
}

void GameWindow::pauseGame() {
  timer_.stop();
}

void GameWindow::resumeGame() {
  timer_.start();
}

void GameWindow::on_debug_rendering_toggled(bool checked) {
  ui->board_widget->setDebugRendering(checked);
}

}  // namespace conquest_ui
