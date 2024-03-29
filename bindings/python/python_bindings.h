// Copyright 2020 The Darwin Neuroevolution Framework Authors.
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

#include <core/darwin.h>
#include <core/utils.h>
#include <core/universe.h>
#include <core/ann_utils.h>
#include <core/evolution.h>

#include <third_party/pybind11/pybind11.h>

#include <memory>
#include <string>
#include <utility>
#include <unordered_map>
#include <functional>
#include <vector>
#include <optional>
using namespace std;

namespace darwin::python {

class PropertySet;
class Universe;

//! Wraps a core::Property pointer (non-onwning)
//!
//! The wrapped core::Property is a reference to a sub-object part of a
//! core::PropertySet. The lifetime of this wrapper must be strictly within
//! the lifetime of the parent core::PropertySet (this can be implemented using
//! py::return_value_policy::reference_internal or py::keep_alive<>)
//!
//! \todo consider a shared_ptr<PropertySet> pointing back to the owner
//!
class Property {
 public:
  explicit Property(core::Property* property);

  string name() const { return property_->name(); }

  string description() const { return property_->description(); }

  string defaultValue() const { return property_->defaultValue(); }

  vector<string> knownValues() const { return property_->knownValues(); }

  PropertySet variant() const;

  //! Automatic conversion to the appropriate Python type
  py::object autoCast() const;

  //! __float__ implementation
  double asFloat() const;

  //! __int__ implementation
  int asInt() const;

  //! __bool__ implementation
  bool asBool() const;

  //! __repr__ implementation
  string repr() const;

  //! __str__ implementation
  string str() const;

 private:
  core::Property* property_ = nullptr;
};

//! Wraps a top-level core::PropertySet pointer (non-owning)
class PropertySet {
 public:
  explicit PropertySet(core::PropertySet* property_set);

  //! to_json()
  string toJson() const;

  //! from_json()
  void fromJson(const string& json_str);

  //! __getattr__ implementation
  Property getAttr(const string& name) const;

  //! __setattr__ with string value
  void setAttrStr(const string& name, const string& value);

  //! __setattr__ with a Property value
  void setAttrProperty(const string& name, const Property* property);

  //! __setattr__ with Python object value
  void setAttrCast(const string& name, py::object value);

  //! __setattr__ with Python boolean value
  void setAttrBool(const string& name, py::bool_ value);

  //! __dir__ implementation
  vector<string> dir() const;

  //! __repr__ implementation
  string repr() const;

 private:
  core::Property* lookupProperty(const string& name) const;

 private:
  core::PropertySet* property_set_ = nullptr;
  unordered_map<string, core::Property*> index_;
};

//! Wrapper for darwin::Domain
class Domain : public core::NonCopyable, public std::enable_shared_from_this<Domain> {
 public:
  explicit Domain(const string& name);

  const string& name() const { return name_; }

  PropertySet config() const { return PropertySet(config_.get()); }

  //! __repr__ implementation
  string repr() const;

  void seal(bool sealed = true);

  void materialize();

  void free();

  darwin::Domain* realDomain() const { return domain_.get(); }

  bool isUsed() const { return used_; }

  void setUsed(bool used) { used_ = used; }

 private:
  string name_;
  darwin::DomainFactory* factory_ = nullptr;
  unique_ptr<core::PropertySet> config_;
  unique_ptr<darwin::Domain> domain_;
  bool sealed_ = false;
  bool used_ = false;
};

//! Wrapper for darwin::Population
class Population : public core::NonCopyable,
                   public std::enable_shared_from_this<Population> {
 public:
  explicit Population(const string& name);

  const string& name() const { return name_; }

  PropertySet config() const { return PropertySet(config_.get()); }

  int size() const { return size_; }

  void setSize(int size);

  //! __repr__ implementation
  string repr() const;

  //! __getitem__ implementation
  unique_ptr<darwin::Genotype> getItem(int index) const;

  void seal(bool sealed = true);

  void materialize(const Domain& domain);

  void free();

  darwin::Population* realPopulation() const { return population_.get(); }

  bool isUsed() const { return used_; }

  void setUsed(bool used) { used_ = used; }

  void updateIndex();

 private:
  string name_;
  darwin::PopulationFactory* factory_ = nullptr;
  unique_ptr<core::PropertySet> config_;
  unique_ptr<darwin::Population> population_;
  vector<size_t> ranking_index_;
  int size_ = 5000;
  bool sealed_ = false;
  bool used_ = false;
};

//! Wrapper for darwin::GenerationSummary
class GenerationSummary {
 public:
  explicit GenerationSummary(const darwin::GenerationSummary& summary)
      : summary_(summary) {}

  int generation() const { return summary_.generation; }
  float bestFitness() const { return summary_.best_fitness; }
  float medianFitness() const { return summary_.median_fitness; }
  float worstFitness() const { return summary_.worst_fitness; }

  optional<PropertySet> calibrationFitness() const;

  unique_ptr<darwin::Genotype> champion() const { return summary_.champion->clone(); }

  string repr() const;

 private:
  darwin::GenerationSummary summary_;
};

//! Wrapper for darwin::Experiment plus evolution driving APIs
class Experiment : public core::NonCopyable,
                   public std::enable_shared_from_this<Experiment> {
 public:
  Experiment(shared_ptr<Domain> domain,
             shared_ptr<Population> population,
             shared_ptr<Universe> universe,
             optional<string> name);

  ~Experiment();

  PropertySet config() { return PropertySet(&config_); }
  PropertySet coreConfig() { return PropertySet(&core_config_); }

  shared_ptr<Domain> domain() const { return domain_; }
  shared_ptr<Population> population() const { return population_; }
  shared_ptr<Universe> universe() const { return universe_; }
  shared_ptr<darwin::EvolutionTrace> trace() const { return trace_; }

  optional<string> name() const { return name_; }

  void setName(const optional<string>& name);

  //! Create the primordial generation and prepare for evolution
  //!
  //! \note All the configuration values are "sealed" to prevent changes
  //!
  void initializePopulation();

  //! Evaluate current population
  GenerationSummary evaluatePopulation();

  //! Create the next generation, using the current fitness values
  //!
  //! \note evaluatePopulation() must be called first
  //!
  void createNextGeneration();

  //! Reset the experiment, "unsealing" config values for editing
  void reset();

  //! __repr__ implementation
  string repr() const;

 private:
  void throwIfDuplicateName(const optional<string>& name) const;

 private:
  darwin::EvolutionConfig config_;
  ann::Config core_config_;

  shared_ptr<Domain> domain_;
  shared_ptr<Population> population_;
  shared_ptr<Universe> universe_;

  optional<string> name_;

  shared_ptr<darwin::Experiment> experiment_;
  shared_ptr<darwin::EvolutionTrace> trace_;
};

//! Wrapper for darwin::Universe
class Universe : public core::NonCopyable, public std::enable_shared_from_this<Universe> {
 public:
  explicit Universe(unique_ptr<darwin::Universe> universe)
      : universe_(std::move(universe)) {}

  ~Universe() { close(); }

  //! Creates a new Darwin experiment
  shared_ptr<Experiment> newExperiment(shared_ptr<Domain> domain,
                                       shared_ptr<Population> population,
                                       optional<string> name);

  //! Closes the Universe object
  void close() { universe_.reset(); }

  //! Returns `true` if the universe object is closed
  bool isClosed() const { return !universe_; }

  //! Returns the universe path name
  string path() const {
    throwIfClosed();
    return universe_->path();
  }

  //! __repr__ implementation
  string repr() const;

  //! Python context manager support
  shared_ptr<Universe> ctxManagerEnter() {
    throwIfClosed();
    return shared_from_this();
  }

  //! Python context manager support
  void ctxManagerExit(py::object /*exc_type*/,
                      py::object /*exc_value*/,
                      py::object /*traceback*/) {
    close();
  }

  darwin::Universe* realUniverse() const;

 private:
  void throwIfClosed() const;

 private:
  unique_ptr<darwin::Universe> universe_;
};

//! Returns the list of available domains
vector<string> availableDomains();

//! Returns the list of available populations
vector<string> availablePopulations();

//! Creates a new universe file
shared_ptr<Universe> createUniverse(const string& path);

//! Opens an existing universe file, or creates a new one if it doesn't exist
shared_ptr<Universe> openUniverse(const string& path);

//! Subscribes a new callback receiving console log output
void addLogger(const function<void(const string&)>& logger);

}  // namespace darwin::python
