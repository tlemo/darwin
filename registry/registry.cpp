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

#include "registry.h"

#include <populations/cgp/cgp.h>
#include <populations/cne/cne.h>
#include <populations/test_population/test_population.h>
#include <populations/neat/neat.h>

#include <domains/conquest/conquest.h>
#include <domains/harvester/harvester.h>
#include <domains/max/max.h>
#include <domains/pong/pong.h>
#include <domains/tic_tac_toe/tic_tac_toe.h>
#include <domains/cart_pole/cart_pole.h>
#include <domains/double_cart_pole/double_cart_pole.h>
#include <domains/unicycle/unicycle.h>
#include <domains/test_domain/test_domain.h>

namespace registry {

void init() {
  // domains
  pong::init();
  find_max_value::init();
  tic_tac_toe::init();
  conquest::init();
  harvester::init();
  cart_pole::init();
  double_cart_pole::init();
  unicycle::init();
  test_domain::init();

  // populations
  cgp::init();
  cne::init();
  test_population::init();
  neat::init();
}

}  // namespace registry
