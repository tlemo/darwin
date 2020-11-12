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

#include <core/darwin.h>
#include <core/properties.h>
#include <core/format.h>
#include <registry/registry.h>

#include <stdlib.h>
#include <sstream>
#include <stdexcept>
using namespace std;

namespace darwin::python {

PropertySet::PropertySet(unique_ptr<core::PropertySet> property_set)
    : property_set_(std::move(property_set)) {
  // create the name-to-property index
  for (auto property : property_set_->properties()) {
    index_[property->name()] = property;
  }
}

string PropertySet::getAttr(const string& name) const {
  return lookupProperty(name)->value();
}

void PropertySet::setAttrStr(const string& name, const string& value) {
  lookupProperty(name)->setValue(value);
}

void PropertySet::setAttrCast(const string& name, py::object value) {
  const string str = py::str(value);

  // automatic conversions from Python sets can result in incorrect results,
  // since the order of the values in the representation is likely different than
  // the original literal, ex:
  //
  //  str({ 10, 20, 5, 1, 100 }) may be '{ 1, 100, 5, 10, 20 }'
  //
  if (!str.empty() && str[0] == '{') {
    const string type_name = py::str(value.get_type().attr("__name__"));
    throw std::runtime_error(
        core::format("Invalid conversion from a '%s' to a property value", type_name));
  }

  setAttrStr(name, str);
}

vector<string> PropertySet::dir() const {
  vector<string> dir;
  for (auto property : property_set_->properties()) {
    dir.push_back(property->name());
  }
  return dir;
}

string PropertySet::repr() const {
  stringstream ss;
  ss << "{\n";
  for (auto property : property_set_->properties()) {
    ss << "  '" << property->name() << "': '" << property->value() << "'  # "
       << property->description();
    const auto known_values = property->knownValues();
    if (!known_values.empty()) {
      ss << ". Valid values: [";
      bool first = true;
      for (const auto& value : known_values) {
        if (!first) {
          ss << ", ";
        }
        ss << "'" << value << "'";
        first = false;
      }
      ss << "]";
    }
    ss << "\n";
  }
  ss << "}\n";
  return ss.str();
}

core::Property* PropertySet::lookupProperty(const string& name) const {
  const auto it = index_.find(name);
  if (it == index_.end()) {
    throw std::runtime_error(core::format("No property with the name '%s'", name));
  }
  return it->second;
}

Domain::Domain(const string& name) : name_(name) {
  if (auto factory = registry()->domains.find(name)) {
    config_ = make_shared<PropertySet>(factory->defaultConfig(ComplexityHint::Balanced));
  } else {
    throw std::runtime_error(core::format("No domain named '%s'", name));
  }
}

string Domain::repr() const {
  return core::format("<darwin.Domain name='%s'>", name_);
}

Population::Population(const string& name) : name_(name) {
  if (auto factory = registry()->populations.find(name)) {
    config_ = make_shared<PropertySet>(factory->defaultConfig(ComplexityHint::Balanced));
  } else {
    throw std::runtime_error(core::format("No population named '%s'", name));
  }
}

string Population::repr() const {
  return core::format("<darwin.Population name='%s'>", name_);
}

Experiment::Experiment(const string& name) {}

shared_ptr<Experiment> Universe::newExperiment() {
  // TODO
  return make_shared<Experiment>("Foo");
}

string Universe::repr() const {
  return isClosed() ? "<darwin.Universe closed>"
                    : core::format("<darwin.Universe path='%s'>", path());
}

void Universe::throwIfClosed() const {
  if (isClosed()) {
    throw std::runtime_error("Attempting to use a closed darwin.Universe object");
  }
}

vector<string> availableDomains() {
  vector<string> domain_names;
  for (const auto& [name, factory] : registry()->domains) {
    domain_names.push_back(name);
  }
  return domain_names;
}

vector<string> availablePopulations() {
  vector<string> population_names;
  for (const auto& [name, factory] : registry()->populations) {
    population_names.push_back(name);
  }
  return population_names;
}

shared_ptr<Universe> createUniverse(const string& path) {
  return make_shared<Universe>(darwin::Universe::create(path));
}

shared_ptr<Universe> openUniverse(const string& path) {
  return make_shared<Universe>(darwin::Universe::open(path));
}

PYBIND11_MODULE(darwin, m) {
  m.doc() = "Darwin Neuroevolution Framework";

  // Darwin initialization
  darwin::init(0, nullptr, getenv("DARWIN_HOME_PATH"));
  registry::init();

  py::class_<PropertySet, shared_ptr<PropertySet>>(m, "PropertySet")
      .def("__getattr__", &PropertySet::getAttr)
      .def("__setattr__", &PropertySet::setAttrStr)
      .def("__setattr__", &PropertySet::setAttrCast)
      .def("__dir__", &PropertySet::dir)
      .def("__repr__", &PropertySet::repr);

  py::class_<Domain, shared_ptr<Domain>>(m, "Domain")
      .def(py::init<const string&>())
      .def_property_readonly("name", &Domain::name)
      .def_property_readonly("config", &Domain::config)
      .def("__repr__", &Domain::repr);

  py::class_<Population, shared_ptr<Population>>(m, "Population")
      .def(py::init<const string&>())
      .def_property_readonly("name", &Population::name)
      .def_property_readonly("config", &Population::config)
      .def("__repr__", &Population::repr);

  py::class_<Experiment, shared_ptr<Experiment>>(m, "Experiment");

  py::class_<Universe, shared_ptr<Universe>>(m, "Universe")
      .def("new_experiment", &Universe::newExperiment)
      .def("close", &Universe::close)
      .def_property_readonly("closed", &Universe::isClosed)
      .def_property_readonly("path", &Universe::path)
      .def("__enter__", &Universe::ctxManagerEnter)
      .def("__exit__", &Universe::ctxManagerExit)
      .def("__repr__", &Universe::repr);

  m.def("available_domains",
        &availableDomains,
        "Returns a list of available domain names.");

  m.def("available_populations",
        &availablePopulations,
        "Returns a list of available population names.");

  m.def("create_universe", &createUniverse, "Creates a new Darwin universe file");
  m.def("open_universe", &openUniverse, "Opens an existing Darwin universe file");
}

}  // namespace darwin::python
