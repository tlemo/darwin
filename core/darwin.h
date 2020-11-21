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

#include "ann_utils.h"
#include "utils.h"
#include "modules.h"
#include "properties.h"
#include "stringify.h"
#include "universe.h"

#include <third_party/json/json.h>
using nlohmann::json;

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <utility>
#include <vector>
using namespace std;

class QWidget;

namespace darwin {

//! A generic hint for the initial population & domain setup
enum class ComplexityHint {
  Minimal,   //!< bare minimum, fast but poor results (mostly useful for testing)
  Balanced,  //!< best guess for the ideal performance/results trade-offs
  Extra,     //!< might produce more sophisticated solutions, but very slow
};

inline auto customStringify(core::TypeTag<ComplexityHint>) {
  static auto stringify = new core::StringifyKnownValues<ComplexityHint>{
    { ComplexityHint::Minimal, "minimal" },
    { ComplexityHint::Balanced, "balanced" },
    { ComplexityHint::Extra, "extra" },
  };
  return stringify;
}

//! The interface to the [Phenotype](https://en.wikipedia.org/wiki/Phenotype)
//!
//! A brain is the phenotype, "grown" from a Genotype. It is the functional
//! model used to evaluate a phenotype's fitness.
//! 
//! \sa Genotype
//!
class Brain {
 public:
  virtual ~Brain() = default;

  //! Sets the value for one of the brain's inputs
  virtual void setInput(int index, float value) = 0;
  
  //! Returns the value of one of the outputs
  //! \note Outputs can be any floating-point value except for NaNs.
  virtual float output(int index) const = 0;
  
  //! Evaluates the outputs from the intput values
  virtual void think() = 0;
  
  //! If the concrete brain implementation maintains internal state
  //! (for example LSTM cells), this method resets this state to the initial values
  virtual void resetState() = 0;
};

//! Models the genealogy information of a genotype
//! 
//! \sa Genotype
//! \sa Population
//! 
struct Genealogy {
  //! Name of the genetic operator used to create the genotype from its parents
  string genetic_operator;

  //! The rank indexes of the parents from the previous generation
  vector<int> parents;

  Genealogy() = default;

  Genealogy(const string& genetic_operator, initializer_list<int> parents)
      : genetic_operator(genetic_operator), parents(parents) {}

  void reset() {
    genetic_operator.clear();
    parents.clear();
  }
};

//! The interface to the population-specific "genetic material", the
//! [Genotype](https://en.wikipedia.org/wiki/Genotype)
//! 
//! This is one of the central concepts in Neuroevolution (and Genetic Algorithms in
//! general). A population is a collection of geneotypes, each describing a particular
//! configuration of genetic material. The genotype is the "recipe" for creating Brain
//! instances (the phenotype), which are the actual functional units mapping domain inputs
//! to output values.
//! 
//! Each genotype's fitness is evaluated indirectly through the phenotype (brain) 
//! counterpart: the better the brain performance, relative to a domain specific criteria,
//! the higher the fitness value.
//! 
//! \sa Brain
//! 
class Genotype {
 public:
  //! The evaluated fitness value (see Domain::evaluatePopulation)
  float fitness = 0;
  
  //! Genealogy information
  Genealogy genealogy;

 public:
  virtual ~Genotype() = default;

  //! "Grow" a Brain using the genetic "recipe" encoded in this genotype
  virtual unique_ptr<Brain> grow() const = 0;
  
  //! Returns a clone of this genotype
  virtual unique_ptr<Genotype> clone() const = 0;

  //! Creates a JSON representation
  virtual json save() const = 0;
  
  //! Loads a JSON representation
  virtual void load(const json& json_obj) = 0;

  virtual void reset() {
    fitness = 0;
    genealogy.reset();
  }
};

//! A population implementation encapsulates the fixed-size set of genotypes,
//! together with the rules for creating a new generation from the previous one.
//! 
//! \sa Domain
//! \sa PopulationFactory
//! 
class Population : public core::NonCopyable {
 public:
  Population() = default;
  virtual ~Population() = default;

  //! The fixed number of genotypes in the population
  virtual size_t size() const = 0;

  //! Indexed access to a genotype in the population
  virtual Genotype* genotype(size_t index) = 0;

  //! Indexed access to a genotype in the population
  virtual const Genotype* genotype(size_t index) const = 0;

  //! Return the indexes of the ranked genotypes (sorted from best to worst)
  //! 
  //! The rankings are calculated based on the fitness values assigned to each genotype,
  //! and potentially other criteria internal to the population (for example a population
  //! may break the fitness ties by favoring less complex genotypes)
  //! 
  virtual vector<size_t> rankingIndex() const = 0;

  //! The current generation number
  virtual int generation() const = 0;

  //! Initialize an initial generation
  virtual void createPrimordialGeneration(int population_size) = 0;

  //! Creates a new generation based on the current one
  //!
  //! It assumes that all the genotypes have been evaluated
  //! (see Domain::evaluatePopulation())
  //!
  //! \sa Domain::evaluatePopulation()
  //!
  virtual void createNextGeneration() = 0;

  //! Array subscript operator (required for pp::for_each)
  Genotype* operator[](size_t index) { return genotype(index); }
  const Genotype* operator[](size_t index) const { return genotype(index); }
};

class Domain;

//! Interface to a population factory
class PopulationFactory : public core::ModuleFactory {
 public:
  //! Creates a new population
  virtual unique_ptr<Population> create(const core::PropertySet& config,
                                        const Domain& domain) = 0;

  //! Returns a default set of properties
  virtual unique_ptr<core::PropertySet> defaultConfig(ComplexityHint hint) const = 0;
};

//! Interface to a domain implementation
//! 
//! A domain defines a problem to be solved, including the environment for plugging in
//! brains from a population (see Brain) and the rules for evaluating the fitness of the
//! corresponding genotypes.
//! 
//! The brains are treated as black boxes with a domain-specific, **fixed** number of
//! inputs and outputs (_ie_ once the domain is configured the values for inputs and
//! outputs will not change during a particular evolution run)
//! 
//! \sa Population
//! \sa DomainFactory
//! 
class Domain : public core::NonCopyable {
 public:
  Domain() = default;
  virtual ~Domain() = default;

  //! Number of inputs to a Brain
  virtual size_t inputs() const = 0;
  
  //! Number of outputs from a Brain
  virtual size_t outputs() const = 0;

  //! Assigns fitness values to every genotype
  //! 
  //! Having a good fitness function is a key part of evolutionary algorithms:
  //! - Perhaps obvious, the fitness value should accurately estimate the quality of a
  //!   particular solution
  //! - A "smooth" distribution is preferable since it provides a gradient which can
  //!   guide the incremental search in the solutions space.
  //!   (ex. if most fitness values are 1.0 or 0.0 it's hard to know which genotypes are
  //!   good candidates for reproduction)
  //! 
  //! \returns `true` if the evolution goal was reached
  //! 
  virtual bool evaluatePopulation(Population* population) const = 0;

  //! Optional: additional fitness metrics
  //! (normally not used in the population evaluation, _ie_ a _test set_)
  virtual unique_ptr<core::PropertySet> calibrateGenotype([
      [maybe_unused]] const Genotype* genotype) const {
    return nullptr;
  }
};

//! Interface to the domain factory
class DomainFactory : public core::ModuleFactory {
 public:
  //! Creates a new domain
  virtual unique_ptr<Domain> create(const core::PropertySet& config) = 0;

  //! Returns a default set of properties
  virtual unique_ptr<core::PropertySet> defaultConfig(ComplexityHint hint) const = 0;
};

//! Interface to a domain UI factory
//! 
//! \todo Move to core_ui?
//! 
class DomainUiFactory : public core::ModuleFactory {
 public:
  //! Creates a new sandbox window, if available (or return nullptr)
  virtual QWidget* newSandboxWindow() = 0;
};

//! The available domains and populations
struct Registry {
  //! Registered populations
  core::ImplementationsSet<PopulationFactory> populations;

  //! Registered domains
  core::ImplementationsSet<DomainFactory> domains;

  //! Registered domains UIs
  core::ImplementationsSet<DomainUiFactory> domains_ui;
};

//! Accessor to the Registry singleton
inline Registry* registry() {
  static Registry instance;
  return &instance;
}

//! An experiment configuration
//
//! \sa Experiment
//! 
struct ExperimentSetup : public core::PropertySet {
  PROPERTY(population_size, int, 5000, "Population size");

  PROPERTY(population_name, string, "", "Population name");
  PROPERTY(domain_name, string, "", "Domain name");

  PROPERTY(population_hint,
           ComplexityHint,
           ComplexityHint::Balanced,
           "Population complexity hint");

  PROPERTY(domain_hint,
           ComplexityHint,
           ComplexityHint::Balanced,
           "Domain complexity hint");
};

//! Encapsulates the runtime experiment state
//! 
//! An experiment is a Population / Domain pair together with a set of configuration
//! knobs which define a particular experiment variation.
//! 
//! \sa ExperimentSetup
//! 
class Experiment : public core::NonCopyable {
 public:
  //! Creates a new experiment
  //! (potentially forking from an existing experiment)
  Experiment(const optional<string>& name,
             const ExperimentSetup& setup,
             const optional<db::RowId>& base_variation_id,
             Universe* universe);

  //! Loads an existing experiment
  Experiment(const DbExperiment* db_experiment, Universe* universe);

  //! The experiment's name
  const optional<string>& name() const { return name_; }
  
  //! The experiment's setup values
  const ExperimentSetup* setup() const { return &setup_; }

  //! Accessor to the PopulationFactory for the experiment's population
  PopulationFactory* populationFactory() const { return population_factory_; }

  //! Accessor to the DomainFactory for the experiment's domain
  DomainFactory* domainFactory() const { return domain_factory_; }
  
  //! Accessor to the DomainUiFactory for the experiment's domain UI
  DomainUiFactory* domainUiFactory() const { return domain_ui_factory_; }

  //! Population specific configuration
  core::PropertySet* populationConfig() { return population_config_.get(); }
  const core::PropertySet* populationConfig() const { return population_config_.get(); }

  //! Domain specific configuration
  core::PropertySet* domainConfig() { return domain_config_.get(); }
  const core::PropertySet* domainConfig() const { return domain_config_.get(); }
  
  //! Core configuration values
  core::PropertySet* coreConfig() { return core_config_.get(); }
  const core::PropertySet* coreConfig() const { return core_config_.get(); }

  //! The associated Darwin Universe
  Universe* universe() const { return universe_; }

  //! Universe database Id of the experiment
  db::RowId dbExperimentId() const { return db_experiment_->id; }

  //! Universe database Id of the particular experiment variation
  db::RowId dbVariationId() const { return db_variation_->id; }

  //! Update the configuration modification flag
  //! 
  //! This is intended for configuration editors (or any code which updates configuration
  //! values directly) to keep track of the state of potential changes. If this flag is
  //! set, a new experiment variation will be created when the experiment starts
  //! 
  void setModified(bool modified) { modified_ = modified; }

  //! Notification that an evolution run is about to start
  //! \sa Evolution::newExperiment()
  void prepareForEvolution();
  
  //! Save the experiment & variation state to the Universe database
  void save();

 private:
  void basicSetup(const optional<string>& name, const ExperimentSetup& setup);
  void loadLatestVariation();

 private:
  optional<string> name_;
  ExperimentSetup setup_;

  // configurations
  unique_ptr<core::PropertySet> population_config_;
  unique_ptr<core::PropertySet> domain_config_;
  unique_ptr<core::PropertySet> core_config_;

  // populations, domains factories
  PopulationFactory* population_factory_ = nullptr;
  DomainFactory* domain_factory_ = nullptr;
  DomainUiFactory* domain_ui_factory_ = nullptr;

  // corresponding database objects
  unique_ptr<DbExperiment> db_experiment_;
  unique_ptr<DbExperimentVariation> db_variation_;
  
  // tracks the configuration modification state
  bool modified_ = false;

  Universe* universe_ = nullptr;
};

}  // namespace darwin
