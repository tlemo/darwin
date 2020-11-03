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

#include <core/utils.h>
#include <core/universe.h>

#include <third_party/pybind11/pybind11.h>
namespace py = pybind11;

#include <memory>
#include <string>
#include <utility>
using namespace std;

namespace darwin::python {

class Domain : public core::NonCopyable, public std::enable_shared_from_this<Domain> {
 public:
  explicit Domain(const string& name);
};

class Population : public core::NonCopyable,
                   public std::enable_shared_from_this<Population> {
 public:
  explicit Population(const string& name);
};

class Experiment : public core::NonCopyable,
                   public std::enable_shared_from_this<Experiment> {
 public:
  explicit Experiment(const string& name);
};

class Universe : public core::NonCopyable, public std::enable_shared_from_this<Universe> {
 public:
  explicit Universe(unique_ptr<darwin::Universe> universe)
      : universe_(std::move(universe)) {}

  //! TODO
  shared_ptr<Experiment> newExperiment();

  //! Close the Universe object
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

 private:
  void throwIfClosed() const;

 private:
  unique_ptr<darwin::Universe> universe_;
};

shared_ptr<Universe> createUniverse(const string& path);

shared_ptr<Universe> openUniverse(const string& path);

}  // namespace darwin::python
