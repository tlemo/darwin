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

#include "registry_ui.h"

#include <domains/pong_ui/pong_ui.h>
#include <domains/tic_tac_toe_ui/tic_tac_toe_ui.h>
#include <domains/conquest_ui/conquest_ui.h>
#include <domains/harvester_ui/harvester_ui.h>
#include <domains/cart_pole_ui/cart_pole_ui.h>
#include <domains/double_cart_pole_ui/double_cart_pole_ui.h>

namespace registry_ui {

void init() {
  pong_ui::init();
  tic_tac_toe_ui::init();
  conquest_ui::init();
  harvester_ui::init();
  cart_pole_ui::init();
  double_cart_pole_ui::init();
}

}  // namespace registry_ui
