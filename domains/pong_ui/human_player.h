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

#pragma once

#include "pong_widget.h"

#include <domains/pong/player.h>

namespace pong_ui {

class HumanPlayer : public pong::Player {
 public:
  HumanPlayer(const PongWidget* pong_widget) : pong_widget_(pong_widget) {}

  Action action() override {
    if (pong_widget_->keyState(side_ == Side::Left ? Qt::Key_Q : Qt::Key_P))
      return Action::MoveUp;
    else if (pong_widget_->keyState(side_ == Side::Left ? Qt::Key_A : Qt::Key_L))
      return Action::MoveDown;
    else
      return Action::None;
  }

  string name() const override { return "Human"; }

 private:
  const PongWidget* pong_widget_ = nullptr;
};

}  // namespace pong_ui
