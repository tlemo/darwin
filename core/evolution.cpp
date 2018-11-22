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

#include "evolution.h"
#include "logging.h"
#include "scope_guard.h"

#include <assert.h>
#include <math.h>
#include <time.h>
#include <fstream>
#include <limits>
#include <memory>
#include <sstream>
#include <system_error>
#include <thread>
using namespace std;

namespace darwin {

ProgressMonitor* ProgressManager::progress_monitor_ = nullptr;

GenerationSummary::GenerationSummary(const Population* population,
                                     shared_ptr<core::PropertySet> calibration_fitness)
    : calibration_fitness(calibration_fitness) {
  size_t size = population->size();
  CHECK(size > 0);

  generation = population->generation();
  best_fitness = population->genotype(0)->fitness;
  median_fitness = population->genotype(size / 2)->fitness;
  worst_fitness = population->genotype(size - 1)->fitness;
  champion = population->genotype(0)->clone();
}

EvolutionTrace::EvolutionTrace(const Evolution* evolution) : evolution_(evolution) {
  auto& experiment = evolution_->experiment();
  auto universe = experiment.universe();
  auto evolution_config = evolution_->config().toJson().dump(2);
  db_trace_ = universe->newTrace(experiment.dbVariationId(), evolution_config);
}

int EvolutionTrace::size() const {
  unique_lock<mutex> guard(lock_);
  return int(generations_.size());
}

GenerationSummary EvolutionTrace::generationSummary(int generation) const {
  unique_lock<mutex> guard(lock_);
  if (generation < 0 || generation >= int(generations_.size()))
    throw core::Exception("Generation %d is not available", generation);
  return generations_[generation];
}

vector<CompressedFitnessValue> compressFitness(const Population* population) {
  // max allowed relative deviation from the actual value
  constexpr float kMaxDeviation = 0.01f;

  vector<CompressedFitnessValue> compressed_values;

  const int raw_size = int(population->size());
  CHECK(raw_size > 0);

  int last_sample_index = 0;
  float last_sample_value = population->genotype(last_sample_index)->fitness;
  compressed_values.emplace_back(last_sample_index, last_sample_value);

  for (int i = 2; i < raw_size; ++i) {
    const float value = population->genotype(i)->fitness;

    // can we extend the current compressed set with the new value?
    // (only if the straight line between it and the last sample is a valid
    //  aproximation of all the intermediate values)
    const int prev_index = i - 1;
    const float prev_value = population->genotype(prev_index)->fitness;

    const float m = float(value - last_sample_value) / (i - last_sample_index);
    CHECK(m <= 0);

    const float aprox_value = (prev_index - last_sample_index) * m + last_sample_value;
    const float relative_dev = fabs((aprox_value - prev_value) / prev_value);

    if (relative_dev > kMaxDeviation) {
      CHECK(prev_index > last_sample_index);
      last_sample_index = prev_index;
      last_sample_value = prev_value;
      compressed_values.emplace_back(last_sample_index, last_sample_value);
    }
  }

  if (raw_size > 1) {
    // append the last real value
    last_sample_index = raw_size - 1;
    last_sample_value = population->genotype(last_sample_index)->fitness;
    compressed_values.emplace_back(last_sample_index, last_sample_value);
  }

  CHECK(!compressed_values.empty());
  return compressed_values;
}

GenerationSummary EvolutionTrace::addGeneration(
    const Population* population,
    shared_ptr<core::PropertySet> calibration_fitness,
    const EvolutionStage& top_stage) {
  GenerationSummary summary(population, calibration_fitness);

  // record the generation summary
  {
    unique_lock<mutex> guard(lock_);
    CHECK(summary.generation == int(generations_.size()));
    generations_.push_back(summary);
  }

  const EvolutionConfig& config = evolution_->config();

  // save the generation results
  DbGeneration db_generation;
  db_generation.trace_id = db_trace_->id;
  db_generation.generation = summary.generation;

  json json_summary;
  json json_details;

  json_summary["best_fitness"] = summary.best_fitness;
  json_summary["median_fitness"] = summary.median_fitness;
  json_summary["worst_fitness"] = summary.worst_fitness;

  if (summary.calibration_fitness) {
    json json_calibration;
    for (auto property : summary.calibration_fitness->properties())
      json_calibration[property->name()] = property->nativeValue<float>();
    json_summary["calibration_fitness"] = json_calibration;
  }

  // detailed fitness information
  switch (config.fitness_information) {
    case FitnessInfoKind::SamplesOnly:
      // nothing to do, we always save the samples
      break;

    case FitnessInfoKind::FullCompressed: {
      // compressed fitness
      json json_compressed_fitness;
      for (const auto& compressed_value : compressFitness(population)) {
        json_compressed_fitness.push_back(
            { compressed_value.index, compressed_value.value });
      }
      json_details["compressed_fitness"] = json_compressed_fitness;
    } break;

    case FitnessInfoKind::FullRaw: {
      // capture all fitness values (ranked)
      json json_full_fitness;
      for (size_t i = 0; i < population->size(); ++i)
        json_full_fitness.push_back(population->genotype(i)->fitness);
      json_details["full_fitness"] = json_full_fitness;
    } break;

    default:
      FATAL("Unexpected fitness information kind");
  }

  // capture genealogy information
  if (config.save_genealogy) {
    json json_full_genealogy;
    for (size_t i = 0; i < population->size(); ++i) {
      const auto& genealogy = population->genotype(i)->genealogy;
      json json_genealogy_entry;
      if (!genealogy.genetic_operator.empty())
        json_genealogy_entry[genealogy.genetic_operator] = genealogy.parents;
      json_full_genealogy.push_back(json_genealogy_entry);
    }
    json_details["genealogy"] = json_full_genealogy;
  }

  // champion genotype
  if (config.save_champion_genotype) {
    json json_genotypes;
    json_genotypes["champion"] = summary.champion->save();
    db_generation.genotypes = json_genotypes.dump();
  }

  // generation runtime profile
  json json_profile;
  json_profile["elapsed"] = top_stage.elapsed();
  switch (config.profile_information) {
    case ProfileInfoKind::GenerationOnly:
      break;

    case ProfileInfoKind::AllStages:
      json_profile["stages"] = top_stage;
      break;

    default:
      FATAL("unexpected profile kind");
  }
  db_generation.profile = json_profile.dump();

  // summary
  CHECK(!json_summary.empty());
  db_generation.summary = json_summary.dump();

  // details
  if (!json_details.empty()) {
    db_generation.details = json_details.dump();
  }

  // save the new generation to the universe database
  auto universe = evolution_->experiment().universe();
  universe->newGeneration(db_generation);

  return summary;
}

void Evolution::init() {
  new std::thread(&Evolution::mainThread, evolution());
}

bool Evolution::newExperiment(shared_ptr<Experiment> experiment,
                              const EvolutionConfig& config) {
  core::log("New experiment (population size = %d)\n\n",
            experiment->setup()->population_size);

  CHECK(config.max_generations >= 0);
  
  {
    unique_lock<mutex> guard(lock_);

    if (state_ != State::Initializing) {
      core::log("Invalid state, can't start a new experiment\n\n");
      return false;
    }

    // sanity checks (make sure we have a clean state)
    CHECK(stage_stack_.empty());

    config_.copyFrom(config);

    CHECK(experiment_ == nullptr);
    experiment_ = experiment;
    experiment_->prepareForEvolution();

    // setup the shared ANN library
    ann::g_config.copyFrom(*experiment->coreConfig());

    // setup the domain
    auto domain_factory = experiment->domainFactory();
    domain_ = domain_factory->create(*experiment->domainConfig());

    // setup the population
    auto population_factory = experiment->populationFactory();
    population_ = population_factory->create(*experiment_->populationConfig(), *domain_);

    trace_ = make_shared<EvolutionTrace>(this);

    state_ = State::Paused;
    state_cv_.notify_all();
  }

  events.publish(EventFlag::ProgressUpdate | EventFlag::StateChanged |
                 EventFlag::NewExperiment);
  return true;
}

void Evolution::mainThread() {
  {
    unique_lock<mutex> guard(lock_);
    CHECK(main_thread_id_ == thread::id());
    main_thread_id_ = std::this_thread::get_id();
  }

  // the "evolution as a service" loop
  for (;;) {
    bool canceled = false;
    
    try {
      evolutionCycle();
      core::log("\nEvolution complete.\n\n");
    } catch (const pp::CanceledException&) {
      core::log("\nRestarting the evolution lifecycle...\n\n");
      canceled = true;
    }
    
    // stop the evolution
    {
      unique_lock<mutex> guard(lock_);
      CHECK(!canceled || state_ == State::Canceling);
      state_ = State::Stopped;
      state_cv_.notify_all();
    }

    events.publish(EventFlag::StateChanged | EventFlag::EndEvolution);
  }
}

void Evolution::evolutionCycle() {
  checkpoint();

  CHECK(experiment_ != nullptr);
  CHECK(domain_);
  CHECK(population_);

  EvolutionStage last_top_stage;

  // TODO: this is an awkward way to capture the top stage, revisit
  auto stages_subscription =
      top_stages.subscribe([&](const EvolutionStage& stage) { last_top_stage = stage; });

  SCOPE_EXIT { top_stages.unsubscribe(stages_subscription); };

  core::log("\nEvolution started:\n\n");

  // main evolution loop
  for (int generation = 0; generation < config_.max_generations; ++generation) {
    shared_ptr<core::PropertySet> calibration_fitness;

    // explicit scope for the top generation stage
    {
      StageScope stage(
          "Evolve one generation", 0, EvolutionStage::Annotation::Generation);

      // create the generation's genotypes
      if (generation == 0) {
        population_->createPrimordialGeneration(experiment_->setup()->population_size);
      } else {
        population_->createNextGeneration();
      }
      
      // TODO: remove generation tracking from darwin::Population
      CHECK(population_->generation() == generation);

      // domain specific evaluation of the genotypes
      if (domain_->evaluatePopulation(population_.get()))
        break;

      // rank the genotypes
      population_->rankGenotypes();

      // extra fitness values (optional)
      const Genotype* champion = population_->genotype(0);
      calibration_fitness = domain_->calibrateGenotype(champion);
    }

    // record the generation
    auto summary =
        trace_->addGeneration(population_.get(), calibration_fitness, last_top_stage);

    // publish the generation results
    generation_summary.publish(summary);
    events.publish(EventFlag::EndGeneration);
    checkpoint();
  }
}

void Evolution::checkpoint() {
  unique_lock<mutex> guard(lock_);

  while (state_ != State::Running) {
    // handle pause requests (Pausing -> Paused)
    if (state_ == State::Pausing) {
      state_ = State::Paused;
      state_cv_.notify_all();

      // annotate the current stage, if any
      if (!stage_stack_.empty())
        stage_stack_.back().addAnnotations(EvolutionStage::Annotation::Paused);

      guard.unlock();
      events.publish(EventFlag::StateChanged);
      guard.lock();

      continue;
    } else if (state_ == State::Canceling) {
      // annotate the current stage, if any
      if (!stage_stack_.empty())
        stage_stack_.back().addAnnotations(EvolutionStage::Annotation::Canceled);

      throw pp::CanceledException();
    }

    state_cv_.wait(guard);
  }
}

void Evolution::beginStage(const string& name, size_t size, uint32_t annotations) {
  // must be called on the main thread
  CHECK(main_thread_id_ == std::this_thread::get_id());

  {
    unique_lock<mutex> guard(lock_);
    stage_stack_.emplace_back(name, size, annotations);
    stage_stack_.back().start();
  }

  events.publish(EventFlag::StateChanged);
}

void Evolution::finishStage(const string& name) {
  // must be called on the main thread
  CHECK(main_thread_id_ == std::this_thread::get_id());

  EvolutionStage stage;
  bool top_stage = false;

  {
    unique_lock<mutex> guard(lock_);

    CHECK(!stage_stack_.empty());
    stage = stage_stack_.back();
    CHECK(stage.name() == name);
    stage.finish();
    stage_stack_.pop_back();

    if (!stage_stack_.empty())
      stage_stack_.back().recordSubStage(stage);
    else
      top_stage = true;
  }

  if ((stage.annotations() & EvolutionStage::Annotation::Canceled) == 0) {
    core::log("Stage complete: %s, %.4f sec\n", stage.name(), stage.elapsed());
  }

  if (top_stage) {
    top_stages.publish(stage);
  }

  events.publish(EventFlag::StateChanged);
}

void Evolution::reportProgress(size_t increment) {
  bool progress_notification = false;

  {
    unique_lock<mutex> guard(lock_);

    CHECK(!stage_stack_.empty());

    auto& current_stage = stage_stack_.back();
    int prev_progress_percent = current_stage.progressPercent();
    current_stage.advanceProgress(increment);
    progress_notification = current_stage.progressPercent() != prev_progress_percent;
  }

  if (progress_notification)
    events.publish(EventFlag::ProgressUpdate);
}

Evolution::Snapshot Evolution::snapshot() const {
  unique_lock<mutex> guard(lock_);
  Snapshot s;
  s.experiment = experiment_;
  s.trace = trace_;
  s.generation = population_ ? population_->generation() : 0;
  s.stage = stage_stack_.empty() ? EvolutionStage() : stage_stack_.back();
  s.state = state_;
  return s;
}

// TODO: consider callback for the pause completition?
void Evolution::pause() {
  bool update = false;

  {
    unique_lock<mutex> guard(lock_);
    if (state_ == State::Running) {
      state_ = State::Pausing;
      state_cv_.notify_all();
      update = true;
    }
  }

  if (update)
    events.publish(EventFlag::StateChanged);
}

void Evolution::run() {
  CHECK(experiment_ != nullptr);

  bool update = false;

  {
    unique_lock<mutex> guard(lock_);
    if (state_ == State::Paused) {
      state_ = State::Running;
      state_cv_.notify_all();
      update = true;
    } else {
      CHECK(state_ == State::Running);
    }
  }

  if (update)
    events.publish(EventFlag::StateChanged);
}

// TODO: consider callback for the reset completition?
bool Evolution::reset() {
  {
    unique_lock<mutex> guard(lock_);

    switch (state_) {
      case State::Initializing:
        // nothing to reset
        return true;

      case State::Paused:
        // requesting cancelation
        state_ = State::Canceling;
        state_cv_.notify_all();
        break;

      case State::Stopped:
        // already stopped
        break;

      default:
        // the evolution must be paused/stopped before it can be reset
        return false;
    }

    // waiting for the cancelation confirmation
    while (state_ != State::Stopped)
      state_cv_.wait(guard);

    // reset the evolution state
    stage_stack_.clear();
    experiment_.reset();
    trace_.reset();
    population_.reset();
    domain_.reset();

    state_ = State::Initializing;
    state_cv_.notify_all();
  }

  core::log("\nThe evolution was reset.\n");
  events.publish(EventFlag::StateChanged | EventFlag::Reset);
  return true;
}

void Evolution::waitForState(Evolution::State target_state) const {
  unique_lock<mutex> guard(lock_);
  while (state_ != target_state)
    state_cv_.wait(guard);
}

EvolutionStage::EvolutionStage(const string& name, size_t size, uint32_t annotations)
    : name_(name), size_(size), annotations_(annotations) {
  CHECK(!name.empty());
}

void EvolutionStage::recordSubStage(const EvolutionStage& stage) {
  annotations_ |= stage.annotations_;
  sub_stages_.push_back(stage);
}

int EvolutionStage::progressPercent() const {
  assert(progress_ <= size_);
  return size_ > 0 ? int(double(progress_) / size_ * 100.0) : 0;
}

void EvolutionStage::advanceProgress(size_t increment) {
  CHECK(increment > 0);
  progress_ += increment;
  CHECK(progress_ <= size_);
}

double EvolutionStage::elapsed() const {
  CHECK(finish_timestamp_ >= start_timestamp_);
  chrono::duration<double> seconds = finish_timestamp_ - start_timestamp_;
  return seconds.count();
}

void EvolutionStage::addAnnotations(uint32_t annotations) {
  annotations_ |= annotations;
}

void to_json(json& json_obj, const EvolutionStage& stage) {
  // TODO: track start/finish timestamps instead of elapsed
  json_obj["name"] = stage.name();
  json_obj["elapsed"] = stage.elapsed();

  const auto& sub_stages = stage.subStages();
  if (!sub_stages.empty()) {
    json_obj["substages"] = sub_stages;
  }
}

}  // namespace darwin
