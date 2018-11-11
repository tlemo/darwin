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

#include "sandbox_window.h"
#include "human_player.h"
#include "new_sandbox_dialog.h"
#include "ui_sandbox_window.h"

#include <domains/tic_tac_toe/ann_player.h>
#include <domains/tic_tac_toe/test_players.h>

#include <core/utils.h>
#include <core/evolution.h>
#include <core/logging.h>

#include <memory>
using namespace std;

namespace tic_tac_toe_ui {

SandboxWindow::SandboxWindow() : ui(new Ui::SandboxWindow) {
  ui->setupUi(this);

  connect(ui->board, &BoardWidget::sigReady, this, &SandboxWindow::play);
}

SandboxWindow::~SandboxWindow() {
  delete ui;
}

static unique_ptr<tic_tac_toe::Player> newPlayer(const QString& type, int generation) {
  if (type == NewSandboxDialog::kAiPlayer) {
    auto snapshot = darwin::evolution()->snapshot();
    auto summary = snapshot.trace->generationSummary(generation);
    auto player = make_unique<tic_tac_toe::AnnPlayer>();
    player->grow(summary.champion.get(), generation);
    return player;
  } else if (type == NewSandboxDialog::kHumanPlayer) {
    return make_unique<HumanPlayer>();
  } else if (type == NewSandboxDialog::kRandomPlayer) {
    return make_unique<tic_tac_toe::RandomPlayer>(false);
  } else if (type == NewSandboxDialog::kInformedRandomPlayer) {
    return make_unique<tic_tac_toe::RandomPlayer>(true);
  } else {
    FATAL("Unexpected player type");
  }
}

bool SandboxWindow::setup() {
  NewSandboxDialog dlg;
  if (dlg.exec() != QDialog::Accepted)
    return false;

  try {
    x_player_ = newPlayer(dlg.xPlayerType(), dlg.xPlayerGeneration());
    o_player_ = newPlayer(dlg.oPlayerType(), dlg.oPlayerGeneration());
  } catch (const exception& e) {
    core::log("Can't create the new sandbox players: %s\n", e.what());
    return false;
  }

  game_.newGame(x_player_.get(), o_player_.get());

  ui->board->setGame(&game_);

  setWindowTitle(QString("%1 vs %2")
                     .arg(QString::fromStdString(x_player_->name()))
                     .arg(QString::fromStdString(o_player_->name())));

  updatePlayerNames();
  play();
  return true;
}

void SandboxWindow::play() {
  // advance the game as many moves as possible
  // (since tic-tac-toe is such a tiny game this
  // should not create problems even on the UI thread)
  while (game_.takeTurn())
    ;
  update();
}

void SandboxWindow::updatePlayerNames() {
  ui->x_player_name->setText(
      QString("X: %1").arg(QString::fromStdString(x_player_->name())));
  ui->o_player_name->setText(
      QString("O: %1").arg(QString::fromStdString(o_player_->name())));
}

void SandboxWindow::on_reset_clicked() {
  game_.reset();
  play();
}

void SandboxWindow::on_flip_sides_clicked() {
  swap(x_player_, o_player_);
  game_.newGame(x_player_.get(), o_player_.get());
  updatePlayerNames();
  play();
}

void SandboxWindow::on_first_move_clicked() {
  game_.historyToFirst();
  update();
}

void SandboxWindow::on_prev_move_clicked() {
  game_.historyToPrevious();
  update();
}

void SandboxWindow::on_next_move_clicked() {
  game_.historyToNext();
  update();
}

void SandboxWindow::on_last_move_clicked() {
  game_.historyToLast();
  update();
}

}  // namespace tic_tac_toe_ui
