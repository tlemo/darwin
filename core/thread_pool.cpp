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

#include "thread_pool.h"
#include "logging.h"

namespace pp {

atomic<ThreadPool*> ParallelForSupport::thread_pool_ = nullptr;

ThreadPool::ThreadPool(int threads_count, Controller* controller)
    : controller_(controller) {
  CHECK(threads_count > 0 || threads_count == kAutoThreadCount);

  if (threads_count == kAutoThreadCount) {
    // number of threads per core
    // TODO: review/evaluate if a different value makes more sense (1.2?)
    constexpr double kThreadsRatio = 1.0;

    const int hw_cores = thread::hardware_concurrency();
    threads_count = max(1, int(hw_cores * kThreadsRatio));
    core::log("New thread pool: autodetected %d core(s), using %d thread(s)\n",
              hw_cores,
              threads_count);
  } else {
    core::log("New thread pool: using %d thread(s)\n", threads_count);
  }

  for (int i = 0; i < threads_count; ++i) {
    worker_threads_.emplace_back(&ThreadPool::workerThread, this);
  }
}

void ThreadPool::processBatch(unique_ptr<WorkBatch> batch) {
  unique_lock<mutex> guard(lock_);

  CHECK(!worker_threads_.empty());

  CHECK(!batch->canceled);
  CHECK(batch->work_left == 0);
  batch->work_left = batch->work_items.size();
  CHECK(batch->work_left > 0);

  if (work_items_.empty())
    queue_cv_.notify_all();

  // push all work items from the batch
  for (auto& work_item : batch->work_items) {
    CHECK(work_item->batch() == batch.get());
    work_items_.push_back(std::move(work_item));
  }
  batch->work_items.clear();

  // wait for the completition of all work items in the batch
  while (batch->work_left > 0)
    results_cv_.wait(guard);

  if (batch->canceled)
    throw CanceledException();
}

void ThreadPool::executeOneItem() {
  auto work_item = acquireWork();

  try {
    if (controller_ != nullptr)
      controller_->checkpoint();

    work_item->execute();
  } catch (const CanceledException&) {
    work_item->batch()->canceled = true;
  }

  finishedWork(work_item->batch());
}

unique_ptr<WorkItem> ThreadPool::acquireWork() {
  unique_lock<mutex> guard(lock_);
  while (work_items_.empty())
    queue_cv_.wait(guard);
  auto work_item = std::move(work_items_.back());
  work_items_.pop_back();
  return work_item;
}

void ThreadPool::finishedWork(WorkBatch* batch) {
  CHECK(batch != nullptr);

  unique_lock<mutex> guard(lock_);
  CHECK(batch->work_left > 0);
  if (--batch->work_left == 0)
    results_cv_.notify_all();
}

void ThreadPool::workerThread() {
  for (;;) {
    executeOneItem();
  }
}

}  // namespace pp
