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

#include "utils.h"
#include "database.h"
#include "properties.h"
#include "stringify.h"

#include <time.h>
#include <mutex>
#include <optional>
using namespace std;

// TODO: test cases

namespace darwin {

//! Base class for all the universe database objects
struct DbUniverseObject {
  //! RowId (unique universe object id)
  db::RowId id = 0;

  //! Object comment/annotation
  optional<string> comment;
  
  //! Object creation timestamp
  time_t timestamp = 0;
};

//! Universe representation of an experiment
struct DbExperiment : public DbUniverseObject {
  //! Experiment name
  //! \todo Remove?
  optional<string> name;
  
  //! Experiment setup values (json)
  string setup;

  //! Pointer to the most recent variation
  optional<db::RowId> last_variation_id;

  //! Time of last activity involving the experiment
  //! (new variation, evolution, ...)
  time_t last_activity_timestamp = 0;
};

//! A variation of an experiment configuration values
//! \sa DbExperiment
struct DbExperimentVariation : public DbUniverseObject {
  //! Previous experiment variation
  //! \note It may point to a branch base variation from a different experiment
  optional<db::RowId> previous_id;

  //! The associated experiment branch
  db::RowId experiment_id = 0;

  //! Variation name
  //! \todo Remove?
  optional<string> name;
  
  //! The experiment configuration values for this variation (json)
  string config;
};

//! A recording of an evolution experiment run
//! \sa DbExperimentVariation
struct DbEvolutionTrace : public DbUniverseObject {
  //! Parent experiment variation
  db::RowId variation_id = 0;

  //! The evolution run configuration (json)
  string config;
};

//! A recording of a particular generation history
//! \sa DbEvolutionTrace
struct DbGeneration : public DbUniverseObject {
  //! The associated evolution trace
  db::RowId trace_id = 0;

  //! Generation number
  int generation = -1;
  
  //! Generation summary (json)
  string summary;
  
  //! Extra details (json)
  optional<string> details;
  
  //! Notable genotypes (json)
  optional<string> genotypes;
  
  //! Runtime profile data (json)
  optional<string> profile;
};

//! The persistent storage for all the experiments and variations
class Universe : public core::NonCopyable {
 public:
  //! Creates a new universe database
  static unique_ptr<Universe> create(const string& path);
  
  //! Opens an existing universe database
  static unique_ptr<Universe> open(const string& path);

  //! The path of this universe database
  string path() const { return path_; }

  //! Creates a new experiment/fork
  unique_ptr<DbExperiment> newExperiment(const optional<string>& name,
                                         const string& setup,
                                         const optional<db::RowId>& base_variation_id);

  //! Loads existing experiment
  unique_ptr<DbExperiment> loadExperiment(db::RowId experiment_id) const;

  //! Returns `true` if there is an experiment with the specified name
  bool findExperiment(const string& name) const;
  
  //! Returns the list of experiments in this universe
  vector<DbExperiment> experimentsList() const;

  //! Creates a new variation
  unique_ptr<DbExperimentVariation> newVariation(db::RowId experiment_id,
                                                 const string& config);

  //! Loads an existing variation
  unique_ptr<DbExperimentVariation> loadVariation(db::RowId variation_id) const;

  //! Create a new evolution trace
  unique_ptr<DbEvolutionTrace> newTrace(db::RowId variation_id,
                                        const string& evolution_config);

  //! Creates a new generation record
  void newGeneration(const DbGeneration& db_generation);

  // yeah, doesn't really belong here, but the standard C++ library
  // support for formatting date/time is still broken (not thread safe)
  string strftime(time_t timestamp, const string& format) const;

 private:
  explicit Universe(const string& path);

  static void initializeUniverse(const string& path);

  db::RowId createVariationHelper(db::RowId experiment_id,
                                  const optional<db::RowId> prev_variation_id,
                                  const string& config);

 private:
  string path_;

  // the database connection is mutable to allow Universe to
  // expose a proper interface (including const methods)
  mutable db::Connection db_;

  // guards all inserts in order to reliably get the last inserted RowId
  mutex db_insert_lock_;
};

}  // namespace darwin
