// Copyright 2019 The Darwin Neuroevolution Framework Authors.
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

#include "domain.h"
#include "scene.h"

#include <core/evolution.h>
#include <core/exception.h>
#include <core/logging.h>
#include <core/parallel_for_each.h>
#include <core/sim/car_controller.h>
#include <core/sim/track.h>

#include <random>
using namespace std;

namespace car_track {

CarTrack::CarTrack(const core::PropertySet& config) {
  config_.copyFrom(config);
  validateConfiguration();

  // setup car configuration
  car_config_.length = config_.car_length;
  car_config_.max_forward_force = config_.max_forward_force;
  car_config_.max_reverse_force = config_.max_reverse_force;
  car_config_.max_steer_angle = config_.max_steer_angle;
  car_config_.tire_traction = config_.tire_traction;
  car_config_.camera = true;
  car_config_.camera_depth = config_.camera_depth;
  car_config_.camera_fov = config_.camera_fov;
  car_config_.camera_resolution = config_.camera_resolution;
  car_config_.touch_sensor = config_.touch_sensor;
  car_config_.touch_resolution = config_.touch_resolution;
  car_config_.accelerometer = config_.accelerometer;
  car_config_.compass = config_.compass;
  car_config_.color = b2Color(0, 1, 0);
}

size_t CarTrack::inputs() const {
  return sim::CarController::inputs(car_config_);
}

size_t CarTrack::outputs() const {
  return sim::CarController::outputs(car_config_);
}

bool CarTrack::evaluatePopulation(darwin::Population* population) const {
  darwin::StageScope stage("Evaluate population");

  const int generation = population->generation();
  core::log("\n. generation %d\n", generation);

  // reset the fitness values
  pp::for_each(*population,
               [&](int, darwin::Genotype* genotype) { genotype->fitness = 0; });

  // evaluate each genotype (over N worlds)
  for (int world_index = 0; world_index < config_.test_worlds; ++world_index) {
    darwin::StageScope stage(
        core::format("World %d/%d", world_index + 1, config_.test_worlds),
        population->size());
    core::log(" ... world %d\n", world_index);

    // create track
    sim::TrackConfig track_config;
    track_config.width = config_.track_width;
    track_config.complexity = config_.track_complexity;
    track_config.resolution = config_.track_resolution;
    track_config.area_width = Scene::kWidth;
    track_config.area_height = Scene::kHeight;
    track_config.curb_width = config_.curb_width;
    track_config.curb_friction = config_.curb_friction;
    track_config.gates = config_.track_gates;
    track_config.solid_gate_posts = config_.solid_gate_posts;
    const auto random_seed = std::random_device{}();
    const sim::Track track(random_seed, track_config);

    pp::for_each(*population, [&](int, darwin::Genotype* genotype) {
      Scene scene(&track, this);
      sim::CarController agent(genotype, scene.car());

      // simulation loop
      for (int step = 0; step < config_.max_steps; ++step) {
        agent.simStep();
        if (!scene.simStep()) {
          break;
        }
      }

      // normalize the fitness to [0, 1], invariant to the number of test worlds
      const float episode_fitness = scene.fitness() / config_.test_worlds;
      genotype->fitness += episode_fitness;

      darwin::ProgressManager::reportProgress();
    });
  }

  core::log("\n");
  return false;
}

// validate the configuration
// (just a few obvious sanity checks for values which would completly break the domain,
// nonsensical configurations are still possible)
void CarTrack::validateConfiguration() {
  if (config_.car_length <= 0)
    throw core::Exception("Invalid configuration: car_length <= 0");
  if (config_.tire_traction <= 0)
    throw core::Exception("Invalid configuration: tire_traction <= 0");
  if (config_.max_forward_force < 0)
    throw core::Exception("Invalid configuration: max_forward_force < 0");
  if (config_.max_reverse_force < 0)
    throw core::Exception("Invalid configuration: max_reverse_force < 0");

  if (config_.camera_fov <= 0 || config_.camera_fov > 360)
    throw core::Exception("Invalid configuration: camera_fov");
  if (config_.camera_resolution < 2)
    throw core::Exception("Invalid configuration: camera_resolution");

  if (config_.track_width <= config_.car_length)
    throw core::Exception("Invalid configuration: track_width too small");
  if (config_.track_complexity < 4)
    throw core::Exception("Invalid configuration: track_complexity < 4");
  if (config_.track_resolution < config_.track_complexity * 10)
    throw core::Exception("Invalid configuration: track_resolution too small");

  if (config_.test_worlds < 1)
    throw core::Exception("Invalid configuration: test_worlds < 1");
  if (config_.max_steps < 1)
    throw core::Exception("Invalid configuration: max_steps < 1");
}

unique_ptr<darwin::Domain> Factory::create(const core::PropertySet& config) {
  return make_unique<CarTrack>(config);
}

unique_ptr<core::PropertySet> Factory::defaultConfig(darwin::ComplexityHint hint) const {
  auto config = make_unique<Config>();
  switch (hint) {
    case darwin::ComplexityHint::Minimal:
      config->test_worlds = 2;
      config->max_steps = 100;
      break;

    case darwin::ComplexityHint::Balanced:
      break;

    case darwin::ComplexityHint::Extra:
      config->test_worlds = 8;
      config->max_steps = 10000;
      config->compass = true;
      config->camera_resolution = 256;
      config->camera_depth = true;
      config->touch_sensor = true;
      config->touch_resolution = 64;
      config->accelerometer = true;
      config->camera_depth = true;
      break;
  }
  return config;
}

}  // namespace car_track
