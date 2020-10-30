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

#include "python_bindings.h"

#include <third_party/pybind11/pybind11.h>
namespace py = pybind11;

namespace darwin::python {

Domain::Domain(const string& name) {}

Population::Population(const string& name) {}

Experiment::Experiment(const string& name) {}

shared_ptr<Experiment> Universe::newExperiment() {
  // TODO
  return make_shared<Experiment>("Foo");
}

shared_ptr<Universe> createUniverse(const string& path) {
  return make_shared<Universe>(darwin::Universe::create(path));
}

shared_ptr<Universe> openUniverse(const string& path) {
  return make_shared<Universe>(darwin::Universe::open(path));
}

PYBIND11_MODULE(darwin, m) {
  m.doc() = "Darwin Neuroevolution Framework";

  py::class_<Domain, shared_ptr<Domain>>(m, "Domain").def(py::init<const string&>());

  py::class_<Population, shared_ptr<Population>>(m, "Population")
      .def(py::init<const string&>());

  py::class_<Experiment, shared_ptr<Experiment>>(m, "Experiment");

  py::class_<Universe, shared_ptr<Universe>>(m, "Universe")
      .def("new_experiment", &Universe::newExperiment);

  m.def("create_universe", &createUniverse);
  m.def("open_universe", &openUniverse);
}

}  // namespace darwin::python
