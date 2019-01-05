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

#include "darwin.h"
#include "pubsub.h"
#include "thread_pool.h"

#include <third_party/json/json.h>
using nlohmann::json;

#include <chrono>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>
using namespace std;

namespace darwin {

class Evolution;

//! Summary of a generation (fitness samples, best genotype, ...)
struct GenerationSummary {
  //! Generation number
  int generation = 0;

  //! Best fitness value
  float best_fitness = 0;

  //! Median fitness value
  float median_fitness = 0;

  //! Worst fitness value
  float worst_fitness = 0;

  //! Calibration fitness values
  shared_ptr<core::PropertySet> calibration_fitness;

  //! Best genotype in the generation
  shared_ptr<Genotype> champion;

  GenerationSummary() = default;

  GenerationSummary(const Population* population,
                    shared_ptr<core::PropertySet> calibration_fitness);
};

//! One fitness data point
struct CompressedFitnessValue {
  //! Genotype index
  int index = -1;
  
  //! Fitness value
  float value = 0;

  CompressedFitnessValue(int index, float value) : index(index), value(value) {}
};

//! The kind of captured fitness data
enum class FitnessInfoKind {
  SamplesOnly,      //!< Just the best/median/worst/calibration fitness values
  FullCompressed,   //!< All the fitness values, compressed
  FullRaw,          //!< All the fitness values, raw
};

inline auto customStringify(core::TypeTag<FitnessInfoKind>) {
  static auto stringify = new core::StringifyKnownValues<FitnessInfoKind>{
    { FitnessInfoKind::SamplesOnly, "samples_only" },
    { FitnessInfoKind::FullCompressed, "full_compressed" },
    { FitnessInfoKind::FullRaw, "full_raw" },
  };
  return stringify;
}

//! The kind of captured profile data
enum class ProfileInfoKind {
  GenerationOnly,   //!< Just the per-generation elapsed timings
  AllStages,        //!< Timings for the full stages tree for a generation
};

inline auto customStringify(core::TypeTag<ProfileInfoKind>) {
  static auto stringify = new core::StringifyKnownValues<ProfileInfoKind>{
    { ProfileInfoKind::GenerationOnly, "generation_only" },
    { ProfileInfoKind::AllStages, "all_stages" },
  };
  return stringify;
}

//! Settings for an evolution experiment run
struct EvolutionConfig : public core::PropertySet {
  PROPERTY(max_generations,
           int,
           1000000,
           "Automatically stop the experiment after the max number of generations");

  PROPERTY(save_champion_genotype,
           bool,
           true,
           "Save the best genotype from each generation");

  PROPERTY(fitness_information,
           FitnessInfoKind,
           FitnessInfoKind::SamplesOnly,
           "What kind of fitness information to save");

  PROPERTY(save_genealogy,
           bool,
           false,
           "Save the genealogy information (can be very large!)");

  PROPERTY(profile_information,
           ProfileInfoKind,
           ProfileInfoKind::GenerationOnly,
           "Performance trace (counters/timings)");
};

vector<CompressedFitnessValue> compressFitness(const Population* population);

//! Tracks the execution of an execution (sub)stage
class EvolutionStage {
  using Clock = std::chrono::steady_clock;

 public:
  //! Stage bit flags
  enum Annotation : uint32_t {
    Paused = 1,      //!< The stage was paused (and resumed)
    Canceled = 2,    //!< The stage was canceled while running
    Generation = 4,  //!< Generation top stage
  };

 public:
  EvolutionStage() = default;

  //! Constructs an stage object
  EvolutionStage(const string& name_, size_t size_, uint32_t annotations);

  //! Marks the start of the stage
  void start() { start_timestamp_ = Clock::now(); }
  
  //! Marks the finish of the stage
  void finish() { finish_timestamp_ = Clock::now(); }

  //! Stage name
  const string& name() const { return name_; }
  
  //! Stage elapsed time in seconds
  double elapsed() const;
  
  //! Progress percent [0..100]
  int progressPercent() const;
  
  //! List of sub-stages, if any
  const vector<EvolutionStage>& subStages() const { return sub_stages_; }

  void recordSubStage(const EvolutionStage& stage);
  void advanceProgress(size_t increment);
  void addAnnotations(uint32_t annotations);
  uint32_t annotations() const { return annotations_; }

  friend void to_json(json& json_obj, const EvolutionStage& stage);

 private:
  string name_;
  size_t size_ = 0;
  size_t progress_ = 0;
  Clock::time_point start_timestamp_;
  Clock::time_point finish_timestamp_;
  uint32_t annotations_ = 0;
  vector<EvolutionStage> sub_stages_;
};

//! Recording of a evolution experiment run
class EvolutionTrace : public core::NonCopyable {
 public:
  EvolutionTrace(const Evolution* evolution);

  //! Number of recorded generations
  int size() const;
  
  //! Indexed access to a recorded generation summary
  GenerationSummary generationSummary(int generation) const;

  GenerationSummary addGeneration(const Population* population,
                                  shared_ptr<core::PropertySet> calibration_fitness,
                                  const EvolutionStage& top_stage);

 private:
  mutable mutex lock_;

  // a brief history of the generations
  vector<GenerationSummary> generations_;

  const Evolution* evolution_ = nullptr;
  unique_ptr<DbEvolutionTrace> db_trace_;
};

//! Interface for monitoring evolution progress
//!
//! \sa ProgressManager
//! \sa Evolution
//! \sa EvolutionStage
//! 
class ProgressMonitor {
 public:
  virtual ~ProgressMonitor() = default;

  //! Stage start notification 
  //! 
  //! \param name - stage name
  //! \param size - the actual size of the stage if known in advance
  //!      (in whaterver units/increments are appropriate)
  //! \param annotations - EvolutionStage::Annotation bit flags
  //! 
  virtual void beginStage(const string& name, size_t size, uint32_t annotations) = 0;
  
  //! Stage finish notification
  virtual void finishStage(const string& name) = 0;

  //! Stage progress notification
  //! 
  //! \param increment - number of units processed, relative to stage size
  //! 
  virtual void reportProgress(size_t increment) = 0;
};

//! Connects the progress updates with a registered progress monitor
//!
//! \note The synchronization between registration and
//!    updates is external (ProgressManager is not responsible of it)
//!
//! \sa ProgressMonitor
//! \sa EvolutionStage
//!
class ProgressManager {
 public:
  //! Reports the start of a stage
  static void beginStage(const string& name, size_t size, uint32_t annotations) {
    if (progress_monitor_ != nullptr) {
      progress_monitor_->beginStage(name, size, annotations);
    }
  }

  //! Reports the finish of a stage
  static void finishStage(const string& name) {
    if (progress_monitor_ != nullptr) {
      progress_monitor_->finishStage(name);
    }
  }

  //! Reports stage progress
  static void reportProgress(size_t increment = 1) {
    if (progress_monitor_ != nullptr) {
      progress_monitor_->reportProgress(increment);
    }
  }

  //! Registers a ProgressMonitor implementation
  static void registerMonitor(ProgressMonitor* monitor) {
    CHECK(progress_monitor_ == nullptr);
    progress_monitor_ = monitor;
  }

 private:
  static ProgressMonitor* progress_monitor_;
};

//! The controller for running evolution experiments
class Evolution : public core::NonCopyable,
                  public pp::Controller,
                  public ProgressMonitor {
  friend Evolution* evolution();

 public:
  //! Evolution state
  enum class State {
    Invalid,        //!< Invalid state tag
    Initializing,   //!< Initializing evolution
    Running,        //!< Evolution experiment is running
    Pausing,        //!< Evolution pause requested
    Paused,         //!< Evolution experiment is paused
    Canceling,      //!< Evolution cancel/stop requested
    Stopped         //!< Evolution experiment was canceled/stopped
  };

  //! Event hints (as bit flags)
  enum EventFlag : uint32_t {
    StateChanged = 1 << 0,    //!< The evolution State has changed
    ProgressUpdate = 1 << 1,  //!< Progress update notification
    EndGeneration = 1 << 2,   //!< Generation end notification
    EndEvolution = 1 << 3,    //!< Evolution run completed successfully
    NewExperiment = 1 << 4,   //!< New experiment notification
    Reset = 1 << 5,           //!< Experiment reset notification
    All = EventFlag(-1),      //!< Combination of all event flags
  };

  //! Evolution events notifications
  core::PubSub<uint32_t> events;
  
  //! Channel for publishing generation summaries
  core::PubSub<GenerationSummary> generation_summary;
  
  //! Channel for publishing the completition of a generation's top stage
  core::PubSub<EvolutionStage> top_stages;

  //! State snapshot of an evolution run
  //! \sa snapshot()
  struct Snapshot {
    //! Evolution state
    State state = State::Invalid;
    
    //! Generation nubmer
    int generation = 0;
    
    //! Currently running stage
    //! (most inner stage if there are nested stages)
    EvolutionStage stage;
    
    //! The associated Experiment instance
    //! \warning This is a shallow copy
    shared_ptr<const Experiment> experiment;
    
    //! The associated EvolutionTrace instance
    //! \warning This is a shallow copy
    shared_ptr<const EvolutionTrace> trace;
    
    //! The associated darwin::Population instance
    //! \warning This is a non-owning pointer and will be invalidated if the
    //!   Snapshot instance outlives the Population instance
    const Population* population = nullptr;

    //! The associated darwin::Domain instance
    //! \warning This is a non-owning pointer and will be invalidated if the
    //!   Snapshot instance outlives the Domain instance
    const Domain* domain = nullptr;
  };

 public:
  static void init();

  //! Sets up a new evolution experiment
  //! 
  //! \param experiment - the Experiment model/state
  //! \param config - evolution runtime settings
  //! 
  //! \todo Copy the experiment instead of shared_ptr?
  //! 
  bool newExperiment(shared_ptr<Experiment> experiment, const EvolutionConfig& config);

  //! Captures a Snapshot of the current evolution state
  Snapshot snapshot() const;

  //! Accessor to the associted Experiment instance
  const Experiment& experiment() const { return *experiment_; }
  
  //! Accessor to the associated EvolutionConfig instance
  const EvolutionConfig& config() const { return config_; }

  //! Start/Resume the evolution
  //! \sa State
  void run();
  
  //! Pause the evolution
  //! \note It just creates a pause request, the actual transition to State::Paused
  //!   happens asynchroniously (normally when the evolution workers reach a checkpoint)
  //! \sa State
  void pause();
  
  //! Resets the evolution (stops execution and resets the state)
  //! \note It just creates a cancelation request, the actual reset
  //!   happens asynchroniously (normally when the evolution workers reach a checkpoint)
  //! \sa State
  bool reset();

  //! Blocks until the requested state is reached
  //! \sa State
  void waitForState(State target_state) const;

 private:
  Evolution() {
    pp::ParallelForSupport::init(this);
    ProgressManager::registerMonitor(this);
  }

  void mainThread();

  void evolutionCycle();

  // pp::Controller interface
  void checkpoint() override;

  // ProgressMonitor interface
  void beginStage(const string& name, size_t size, uint32_t annotations) override;
  void finishStage(const string& name) override;
  void reportProgress(size_t increment = 1) override;

 private:
  std::thread::id main_thread_id_;

  mutable mutex lock_;
  mutable condition_variable state_cv_;

  State state_ = State::Initializing;
  vector<EvolutionStage> stage_stack_;

  EvolutionConfig config_;

  // population & domain
  unique_ptr<Population> population_;
  unique_ptr<Domain> domain_;

  shared_ptr<Experiment> experiment_;
  shared_ptr<EvolutionTrace> trace_;
};

//! Accessor to the Evolution singleton instance
inline Evolution* evolution() {
  static Evolution* instance = new Evolution();
  return instance;
}

//! A scope-based Stage wrapper
class StageScope {
 public:
  //! Starts a new stage, which will be automatically completed at
  //! the end of the current scope
  StageScope(const string& name, size_t size = 0, uint32_t annotations = 0)
      : name_(name) {
    ProgressManager::beginStage(name, size, annotations);
  }

  //! Finishes the stage
  ~StageScope() { ProgressManager::finishStage(name_); }

 private:
  string name_;
};

}  // namespace darwin
