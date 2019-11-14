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
#include <core/sim/drone_controller.h>

#include <random>
using namespace std;

namespace drone_follow {

DroneFollow::DroneFollow(const core::PropertySet& config) {
  config_.copyFrom(config);
  validateConfiguration();
  
  // setup drone configuration
  drone_config_.position = b2Vec2(0, -config_.target_distance);
  drone_config_.radius = config_.drone_radius;
  drone_config_.camera = true;
  drone_config_.camera_depth = config_.camera_depth;
  drone_config_.camera_fov = config_.camera_fov;
  drone_config_.camera_resolution = config_.camera_resolution;
  drone_config_.touch_sensor = config_.touch_sensor;
  drone_config_.touch_resolution = config_.touch_resolution;
  drone_config_.accelerometer = config_.accelerometer;
  drone_config_.compass = config_.compass;
  drone_config_.max_move_force = config_.max_move_force;
  drone_config_.max_rotate_torque = config_.max_rotate_torque;
  drone_config_.lights = config_.drone_lights;
}

size_t DroneFollow::inputs() const {
  return sim::DroneController::inputs(drone_config_);
}

size_t DroneFollow::outputs() const {
  return sim::DroneController::outputs(drone_config_);
}

bool DroneFollow::evaluatePopulation(darwin::Population* population) const {
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

    const auto random_seed = std::random_device{}();

    pp::for_each(*population, [&](int, darwin::Genotype* genotype) {
      Scene scene(random_seed, this);
      sim::DroneController agent(genotype, scene.drone());

      // simulation loop
      for (int step = 0; step < config_.max_steps; ++step) {
        agent.simStep();
        if (!scene.simStep()) {
          break;
        }
      }

      // normalize the fitness to [0, 1], invariant to the number of steps or test worlds
      float episode_fitness = scene.fitness();
      episode_fitness /= config_.max_steps;
      episode_fitness /= config_.test_worlds;
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
void DroneFollow::validateConfiguration() {
  if (config_.drone_radius <= 0)
    throw core::Exception("Invalid configuration: drone_radius <= 0");
  if (config_.max_move_force < 0)
    throw core::Exception("Invalid configuration: max_move_force < 0");
  if (config_.max_rotate_torque < 0)
    throw core::Exception("Invalid configuration: max_rotate_torque < 0");

  if (config_.camera_fov <= 0 || config_.camera_fov > 360)
    throw core::Exception("Invalid configuration: camera_fov");
  if (config_.camera_resolution < 2)
    throw core::Exception("Invalid configuration: camera_resolution");

  if (config_.target_distance < config_.drone_radius * 2)
    throw core::Exception("Invalid configuration: target_distance too close");

  if (config_.test_worlds < 1)
    throw core::Exception("Invalid configuration: test_worlds < 1");
  if (config_.max_steps < 1)
    throw core::Exception("Invalid configuration: max_steps < 1");
}

unique_ptr<darwin::Domain> Factory::create(const core::PropertySet& config) {
  return make_unique<DroneFollow>(config);
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
      config->test_worlds = 10;
      config->max_steps = 10000;
      config->compass = true;
      config->touch_sensor = true;
      config->accelerometer = true;
      config->camera_depth = true;
      break;
  }
  return config;
}

}  // namespace drone_follow
