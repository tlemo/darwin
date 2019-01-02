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

#include "darwin.h"
#include "logging.h"
#include "platform_abstraction_layer.h"
#include "ann_dynamic.h"

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

namespace darwin {

constexpr char kDarwinHome[] = ".darwin";

#ifdef NDEBUG
const char* kBuild = ("RELEASE_BUILD " __DATE__ ", " __TIME__);
#else
const char* kBuild = ("DEBUG_BUILD " __DATE__ ", " __TIME__);
#endif  // NDEBUG

void init(int, char*[], const char* home_path) {
  // make sure Darwin home directory exists and set current directory to it
  // (the explicit home_path is used for setting up the testing environment)
  auto home = (home_path == nullptr) ? pal::userHomePath() : string(home_path);
  const auto darwin_home_path = fs::path(home) / kDarwinHome;
  fs::create_directories(darwin_home_path);
  fs::current_path(darwin_home_path);

  core::initLogging();
  ann::initAnnLibrary();
}

void shutdown() {}

Experiment::Experiment(const optional<string>& name,
                       const ExperimentSetup& setup,
                       const optional<db::RowId>& base_variation_id,
                       Universe* universe)
    : universe_(universe) {
  basicSetup(name, setup);

  // create the universe counterpart
  db_experiment_ =
      universe->newExperiment(name, setup_.toJson().dump(2), base_variation_id);

  // load the most recent experiment variation, if any
  // (for new experiments this can happen if it's a fork of an existing experiment)
  if (db_experiment_->last_variation_id.has_value()) {
    loadLatestVariation();
  }
}

Experiment::Experiment(const DbExperiment* db_experiment, Universe* universe)
    : universe_(universe) {
  darwin::ExperimentSetup setup;
  setup.fromJson(json::parse(db_experiment->setup));

  basicSetup(db_experiment->name, setup);

  db_experiment_ = make_unique<DbExperiment>(*db_experiment);

  // load the most recent experiment variation, if any
  if (db_experiment_->last_variation_id.has_value())
    loadLatestVariation();
}

void Experiment::basicSetup(const optional<string>& name, const ExperimentSetup& setup) {
  CHECK(!db_experiment_);
  CHECK(!db_variation_);

  name_ = name;

  // setup the domain
  domain_factory_ = registry()->domains.find(setup.domain_name);
  if (domain_factory_ == nullptr)
    throw core::Exception("Can't create domain: '%s'", setup.domain_name);
  domain_config_ = domain_factory_->defaultConfig(setup.domain_hint);

  // the domain UI may be nullptr if not available
  domain_ui_factory_ = registry()->domains_ui.find(setup.domain_name);

  // setup the population
  population_factory_ = registry()->populations.find(setup.population_name);
  if (population_factory_ == nullptr)
    throw core::Exception("Can't create population: '%s'", setup.population_name);
  population_config_ = population_factory_->defaultConfig(setup.population_hint);

  // core (shared) config
  core_config_ = make_unique<ann::Config>();

  setup_.copyFrom(setup);
}

void Experiment::loadLatestVariation() {
  auto db_variation = universe_->loadVariation(db_experiment_->last_variation_id.value());
  CHECK(db_variation->experiment_id == db_experiment_->id);

  auto json_config = json::parse(db_variation->config);
  core_config_->fromJson(json_config.at("core"));
  domain_config_->fromJson(json_config.at("domain"));
  population_config_->fromJson(json_config.at("population"));

  db_variation_ = std::move(db_variation);
  modified_ = false;
}

void Experiment::prepareForEvolution() {
  // don't allow any more modifications
  core_config_->seal();
  domain_config_->seal();
  population_config_->seal();

  save();
}

void Experiment::save() {
  if (!db_variation_ || modified_) {
    core::log("Creating a new experiment variation ...\n");
    json json_config;
    json_config["core"] = core_config_->toJson();
    json_config["domain"] = domain_config_->toJson();
    json_config["population"] = population_config_->toJson();
    db_variation_ = universe_->newVariation(db_experiment_->id, json_config.dump(2));
    modified_ = false;
  }
}

}  // namespace darwin
