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

#include "universe.h"
#include "format.h"
#include "logging.h"

#include <optional>
using namespace std;

namespace darwin {

constexpr int32_t kSqlApplicationId = 0x47414e4e;
constexpr int32_t kSqlFormatVersion = 1;

unique_ptr<Universe> Universe::create(const string& path) {
  core::log("Creating new universe: '%s'...\n", path.c_str());
  initializeUniverse(path);
  return unique_ptr<Universe>(new Universe(path));
}

unique_ptr<Universe> Universe::open(const string& path) {
  core::log("Opening universe: '%s'...\n", path.c_str());
  return unique_ptr<Universe>(new Universe(path));
}

// open an existing universe (or throws)
Universe::Universe(const string& path)
    : path_(path), db_(path, db::OpenMode::ExistingDatabase) {
  if (db_.exec<int>("pragma application_id").singleValue() != kSqlApplicationId)
    throw core::Exception("Invalid universe database (application id)");

  if (db_.exec<int>("pragma user_version").singleValue() != kSqlFormatVersion)
    throw core::Exception("Incompatible universe format");

  db_.exec("pragma quick_check");
}

void Universe::initializeUniverse(const string& path) {
  db::Connection new_db(path, db::OpenMode::CreateNew);
  CHECK(new_db.exec<int>("pragma application_id").singleValue() == 0);

  // setup the database schema
  db::TransactionScope transaction(new_db);

  new_db.exec(core::format("pragma application_id = 0x%x", kSqlApplicationId));
  new_db.exec(core::format("pragma user_version = %d", kSqlFormatVersion));

  // TODO: a way to define both the db schema and the db objects simultaneously

  new_db.exec(R"(create table Experiment(
        id integer primary key,
        comment text,
        timestamp int,
        name text unique,
        last_variation_id int,
        last_activity_timestamp int,
        setup text))");

  new_db.exec(R"(create table Variation(
        id integer primary key,
        comment text,
        timestamp int,
        previous_id int,
        experiment_id int,
        name text,
        config text))");

  new_db.exec(R"(create table Trace(
        id integer primary key,
        comment text,
        timestamp int,
        variation_id int,
        evolution_config text))");

  new_db.exec(R"(create table Generation(
        id integer primary key,
        timestamp int,
        trace_id int,
        generation int,
        summary text,
        details text,
        genotypes text,
        profile text))");

  transaction.commit();
}

// 1. this is a private helper which must be called under the db_insert_lock_
// 2. it doesn't create any transactions itself, so it can, and should be wrapped
//    in a caller transaction (since it updates teh parent experiment as well)
db::RowId Universe::createVariationHelper(db::RowId experiment_id,
                                          const optional<db::RowId> prev_variation_id,
                                          const string& config) {
  int64_t now_timestamp = time(nullptr);

  // insert the new variation
  db_.exec(
      R"(insert into Variation(
            timestamp,
            experiment_id,
            previous_id,
            config)
        values(?, ?, ?, ?))",
      now_timestamp,
      experiment_id,
      prev_variation_id,
      config);

  auto new_variation_id = db_.lastInsertRowId();

  // update the experiment's last_variation_id & last_activity_timestamp
  db_.exec(
      R"(update experiment set
            last_variation_id = ?,
            last_activity_timestamp = ?
        where id = ?)",
      new_variation_id,
      now_timestamp,
      experiment_id);

  return new_variation_id;
}

unique_ptr<DbExperiment> Universe::newExperiment(
    const optional<string>& name,
    const string& setup,
    const optional<db::RowId>& base_variation_id) {
  unique_lock<mutex> guard(db_insert_lock_);

  int64_t now_timestamp = time(nullptr);

  auto new_experiment = make_unique<DbExperiment>();
  new_experiment->name = name;
  new_experiment->setup = setup;
  new_experiment->timestamp = now_timestamp;
  new_experiment->last_variation_id = nullopt;
  new_experiment->last_activity_timestamp = now_timestamp;

  // transaction scope
  // (the experiment and base variation cloning must be atomic)
  {
    db::TransactionScope transaction(db_, db::TransactionOption::Immediate);

    // create the experiment entry
    db_.exec(
        R"(insert into Experiment(
                timestamp,
                last_variation_id,
                last_activity_timestamp,
                name,
                setup) 
            values(?, ?, ?, ?, ?))",
        now_timestamp,
        base_variation_id,
        now_timestamp,
        name,
        setup);

    new_experiment->id = db_.lastInsertRowId();

    // clone the base variation as the tip of the new experiment
    if (base_variation_id.has_value()) {
      auto base_variation = loadVariation(base_variation_id.value());
      new_experiment->last_variation_id = createVariationHelper(
          new_experiment->id, base_variation_id, base_variation->config);
    }

    transaction.commit();
  }

  return new_experiment;
}

unique_ptr<DbExperiment> Universe::loadExperiment(db::RowId experiment_id) const {
  auto results = db_.exec<db::RowId, string, int64_t, string, string, db::RowId, int64_t>(
      R"(select
                id,
                comment,
                timestamp,
                name,
                setup,
                last_variation_id,
                last_activity_timestamp
            from experiment
                where id = ?)",
      experiment_id);

  CHECK(results.size() == 1);
  const auto& [id, comment, timestamp, name, setup, last_variation_id,
               last_activity_timestamp] = results[0];

  auto db_experiment = make_unique<DbExperiment>();
  db_experiment->id = id.value();
  db_experiment->comment = comment;
  db_experiment->timestamp = timestamp.value();
  db_experiment->name = name;
  db_experiment->setup = setup.value();
  db_experiment->last_variation_id = last_variation_id;
  db_experiment->last_activity_timestamp = last_activity_timestamp.value();

  CHECK(db_experiment->id != 0);

  return db_experiment;
}

bool Universe::findExperiment(const string& name) const {
  return !db_.exec("select null from Experiment where name = ?", name).empty();
}

// TODO: consider using an incremental iterator
vector<DbExperiment> Universe::experimentsList() const {
  auto results = db_.exec<db::RowId, string, int64_t, string, string, db::RowId, int64_t>(
      R"(select
                id,
                comment,
                timestamp,
                name,
                setup,
                last_variation_id,
                last_activity_timestamp
            from experiment)");

  vector<DbExperiment> experiments;
  for (const auto& [id, comment, timestamp, name, setup, last_variation_id,
                    last_activity_timestamp] : results) {
    DbExperiment db_experiment;
    db_experiment.id = id.value();
    db_experiment.comment = comment;
    db_experiment.timestamp = timestamp.value();
    db_experiment.name = name;
    db_experiment.setup = setup.value();
    db_experiment.last_variation_id = last_variation_id;
    db_experiment.last_activity_timestamp = last_activity_timestamp.value();
    CHECK(db_experiment.id != 0);
    experiments.push_back(db_experiment);
  }

  return experiments;
}

unique_ptr<DbExperimentVariation> Universe::newVariation(db::RowId experiment_id,
                                                         const string& config) {
  unique_lock<mutex> guard(db_insert_lock_);

  auto new_variation = make_unique<DbExperimentVariation>();
  new_variation->experiment_id = experiment_id;
  new_variation->config = config;

  // transaction scope
  // (the experiment's last_variation_id read/update must be atomic)
  {
    db::TransactionScope transaction(db_, db::TransactionOption::Immediate);

    // get the previous variation
    auto last_variation_id =
        db_.exec<db::RowId>("select last_variation_id from experiment where id = ?",
                            experiment_id)
            .singleValue();

    new_variation->id = createVariationHelper(experiment_id, last_variation_id, config);

    transaction.commit();
  }

  return new_variation;
}

unique_ptr<DbExperimentVariation> Universe::loadVariation(db::RowId variation_id) const {
  auto results =
      db_.exec<db::RowId, string, int64_t, db::RowId, db::RowId, string, string>(
          R"(select
            id,
            comment,
            timestamp,
            previous_id,
            experiment_id,
            name,
            config
        from variation
            where id = ?)",
          variation_id);

  CHECK(results.size() == 1);
  const auto& [id, comment, timestamp, previous_id, experiment_id, name, config] =
      results[0];

  auto db_variation = make_unique<DbExperimentVariation>();
  db_variation->id = id.value();
  db_variation->comment = comment;
  db_variation->timestamp = timestamp.value();
  db_variation->previous_id = previous_id;
  db_variation->experiment_id = experiment_id.value();
  db_variation->name = name;
  db_variation->config = config.value();

  CHECK(db_variation->id != 0);
  CHECK(db_variation->experiment_id != 0);

  return db_variation;
}

unique_ptr<DbEvolutionTrace> Universe::newTrace(db::RowId variation_id,
                                                const string& evolution_config) {
  unique_lock<mutex> guard(db_insert_lock_);

  auto new_trace = make_unique<DbEvolutionTrace>();
  new_trace->variation_id = variation_id;

  // transaction scope
  {
    db::TransactionScope transaction(db_, db::TransactionOption::Immediate);

    // insert the new trace
    db_.exec(
        R"(insert into Trace(
                timestamp,
                variation_id,
                evolution_config)
            values(?, ?, ?))",
        int64_t(time(nullptr)),
        variation_id,
        evolution_config);

    new_trace->id = db_.lastInsertRowId();

    // update the parent experiment last_activity_timestamp
    int64_t now_timestamp = time(nullptr);
    db_.exec(
        R"(update experiment set
                last_activity_timestamp = ?
            where id in
                (select experiment_id
                from variation
                where variation.id = ?))",
        now_timestamp,
        variation_id);

    transaction.commit();
  }

  return new_trace;
}

void Universe::newGeneration(const DbGeneration& db_generation) {
  unique_lock<mutex> guard(db_insert_lock_);

  db_.exec(
      R"(insert into Generation(
            timestamp,
            trace_id,
            generation,
            summary,
            details,
            genotypes,
            profile)
        values(?, ?, ?, ?, ?, ?, ?))",
      int64_t(time(nullptr)),
      db_generation.trace_id,
      db_generation.generation,
      db_generation.summary,
      db_generation.details,
      db_generation.genotypes,
      db_generation.profile);
}

string Universe::strftime(time_t timestamp, const string& format) const {
  const auto& result = db_.exec<string>(
      "select strftime(?, ?, 'unixepoch', 'localtime')", format, int64_t(timestamp));
  return result.singleValue().value();
}

}  // namespace darwin
