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

#include <assert.h>
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
using namespace std;

namespace pp {

struct WorkBatch;

//! Generic work item interface
//! \sa WorkBatch
class WorkItem {
 public:
  WorkItem(WorkBatch* batch) : batch_(batch) {}
  virtual ~WorkItem() = default;
  
  //! Runs the work item implementation
  virtual bool execute() = 0;

  //! The associated (parent) WorkBatch
  WorkBatch* batch() const { return batch_; }

 private:
  // parent work batch
  WorkBatch* const batch_ = nullptr;
};

// A work item wrapping a closure object (aka function objects)
template <class Body>
class ClosureWorkItem : public WorkItem {
 public:
  ClosureWorkItem(WorkBatch* batch, const Body& item_body)
      : WorkItem(batch), item_body_(item_body) {}

  bool execute() override {
    item_body_();
    return true;
  }

 private:
  Body item_body_;
};

//! A collection of work items to be processed in a fork/join fashion
//! 
//! The uses must follow the pattern: 
//!  1. Create an WorkBatch instance
//!  2. Populate it using pushWork()
//!  3. Call ThreadPool::processBatch()
//! 
//! \sa WorkItem
//! \sa ThreadPool
//! 
struct WorkBatch {
  //! The set of work items to be processed
  vector<unique_ptr<WorkItem>> work_items;

  //! Used by the work queue to track the progress
  size_t work_left = 0;

  //! Cancellation support
  atomic<bool> canceled = false;

  //! Appends a new WorkItem
  template <class Body>
  void pushWork(const Body& body) {
    work_items.emplace_back(make_unique<ClosureWorkItem<Body>>(this, body));
  }
};

class CanceledException {};

//! An optional thread pool controller, which can be used to
//! pause/resume/cancel the queued work items
//!
//! \note The controller instance must outlive the associated thread pool
//! 
//! \sa ThreadPool
//!
class Controller {
 public:
  virtual ~Controller() = default;

  //! Controller interface, called from the worker threads.
  //!
  //! Implementations may:
  //! - Return (ie. allow worker threads to run)
  //! - Block (ie. pause worker threads)
  //! - Throw CanceledException
  //!
  virtual void checkpoint() = 0;
};

//! A basic thread pool (managing a fixed number of threads)
//! 
//! \sa WorkItem
//! \sa WorkBatch
//! \sa Controller
//! 
class ThreadPool : public core::NonCopyable {
 public:
  //! Automatic thread count, based on the detected number of cores
  static constexpr int kAutoThreadCount = 0;

 public:
  //! Creates a new thread pool
  //! 
  //! \param threads_count - the count of threads, or kAutoThreadCount
  //! \param controller - an optional thread pool controller
  //! 
  ThreadPool(int threads_count, Controller* controller = nullptr);

  //! Queues the work items in the specified batch and waits for completition
  //! \sa WorkBatch
  void processBatch(unique_ptr<WorkBatch> batch);

  //! The number of threads managed by this thread pool
  int threadsCount() const { return int(worker_threads_.size()); }

 private:
  void executeOneItem();
  unique_ptr<WorkItem> acquireWork();
  void finishedWork(WorkBatch* batch);
  void workerThread();

 private:
  deque<unique_ptr<WorkItem>> work_items_;
  vector<thread> worker_threads_;
  Controller* controller_ = nullptr;

  mutable mutex lock_;
  mutable condition_variable queue_cv_;
  mutable condition_variable results_cv_;
};

class ParallelForSupport {
 public:
  static void init(Controller* controller) {
    auto thread_pool = make_unique<ThreadPool>(ThreadPool::kAutoThreadCount, controller);
    CHECK(thread_pool_.exchange(thread_pool.release()) == nullptr);
  }

  static ThreadPool* threadPool() { return thread_pool_; }

 private:
  static atomic<ThreadPool*> thread_pool_;
};

}  // namespace pp
