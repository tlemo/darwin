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
#include <core/stringify.h>
#include <registry/registry.h>

#include <stdlib.h>
#include <sstream>
#include <stdexcept>
using namespace std;

namespace darwin::python {

Property::Property(core::Property* property) : property_(property) {
  CHECK(property_ != nullptr);
}

PropertySet Property::variant() const {
  if (auto child_property_set = property_->childPropertySet()) {
    return PropertySet(child_property_set);
  } else {
    throw std::runtime_error(core::format(
        "Property '%s' doesn't have a variant property set", property_->name()));
  }
}

pybind11::object Property::autoCast() const {
  const auto value_str = property_->value();

  // boolean?
  try {
    return py::bool_(core::fromString<bool>(value_str));
  } catch(...) {
    // conversion failed, try something else...
  }

  // integer?
  try {
    return py::int_(core::fromString<int>(value_str));
  } catch (...) {
    // conversion failed, try something else...
  }

  // floating point?
  try {
    return py::float_(core::fromString<double>(value_str));
  } catch (...) {
    // conversion failed, try something else...
  }

  // if everything else failed, return a Python string value
  return py::str(value_str);
}

double Property::asFloat() const {
  return core::fromString<double>(property_->value());
}

int Property::asInt() const {
  return core::fromString<int>(property_->value());
}

bool Property::asBool() const {
  return core::fromString<bool>(property_->value());
}

string Property::repr() const {
  return property_->value();
}

string Property::str() const {
  return property_->value();
}

PropertySet::PropertySet(core::PropertySet* property_set) : property_set_(property_set) {
  CHECK(property_set_ != nullptr);
  // create the name-to-property index
  for (auto property : property_set_->properties()) {
    index_[property->name()] = property;
  }
}

Property PropertySet::getAttr(const string& name) const {
  return Property(lookupProperty(name));
}

void PropertySet::setAttrStr(const string& name, const string& value) {
  lookupProperty(name)->setValue(value);
}

void PropertySet::setAttrProperty(const string& name, const Property* property) {
  CHECK(property != nullptr);
  setAttrStr(name, property->repr());
}

void PropertySet::setAttrCast(const string& name, py::object value) {
  const string str = py::repr(value);

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

void PropertySet::setAttrBool(const string& name, pybind11::bool_ value) {
  setAttrStr(name, value ? "true" : "false");
}

vector<string> PropertySet::dir() const {
  vector<string> dir;
  for (auto property : property_set_->properties()) {
    dir.push_back(property->name());
  }
  return dir;
}

static string dumpPropertySet(const core::PropertySet* property_set, int nest_level) {
  stringstream ss;

  const auto indent = [&]() -> stringstream& {
    for (int i = 0; i < nest_level; ++i) {
      ss << "  ";
    }
    return ss;
  };

  indent() << (property_set->sealed() ? "SEALED " : "") << "{\n";
  for (auto property : property_set->properties()) {
    indent() << "  '" << property->name() << "': '" << property->value() << "'  # "
             << property->description();

    // known (valid) values
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

    // child property set (if any)
    if (const auto child_property_set = property->childPropertySet()) {
      ss << dumpPropertySet(child_property_set, nest_level + 2);
    }
  }
  indent() << "}\n";
  return ss.str();
}

string PropertySet::repr() const {
  return dumpPropertySet(property_set_, 0);
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
    config_ = factory->defaultConfig(ComplexityHint::Balanced);
  } else {
    throw std::runtime_error(core::format("No domain named '%s'", name));
  }
}

string Domain::repr() const {
  return core::format("<darwin.Domain name='%s'>", name_);
}

Population::Population(const string& name) : name_(name) {
  if (auto factory = registry()->populations.find(name)) {
    config_ = factory->defaultConfig(ComplexityHint::Balanced);
  } else {
    throw std::runtime_error(core::format("No population named '%s'", name));
  }
}

void Population::setSize(int size) {
  if (sealed_) {
    throw std::runtime_error("Can't set the size of a sealed population");
  }
  if (size < 1) {
    throw std::runtime_error("Invalid population size (must be at least 1)");
  }
  size_ = size;
}

string Population::repr() const {
  return core::format("<darwin.Population name='%s'>", name_);
}

void Population::seal(bool sealed) {
  sealed_ = sealed;
  config_->seal(sealed);
}

Experiment::Experiment(shared_ptr<Domain> domain,
                       shared_ptr<Population> population,
                       shared_ptr<Universe> universe)
    : domain_(std::move(domain)),
      population_(std::move(population)),
      universe_(std::move(universe)) {}

void Experiment::initializePopulation() {
  config_.seal();
  core_config_.seal();
  domain_->seal();
  population_->seal();
}

void Experiment::evaluatePopulation() {}

void Experiment::createNextGeneration() {}

void Experiment::reset() {
  config_.seal(false);
  core_config_.seal(false);
  domain_->seal(false);
  population_->seal(false);
}

string Experiment::repr() const {
  return core::format("<darwin.Experiment domain='%s', population='%s'>",
                      domain_->name(),
                      population_->name());
}

shared_ptr<Experiment> Universe::newExperiment(shared_ptr<Domain> domain,
                                               shared_ptr<Population> population) {
  return make_shared<Experiment>(domain, population, shared_from_this());
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

  // pybind11 ownership policy used to keep the parent alive when returning a sub-object
  // (https://pybind11.readthedocs.io/en/stable/advanced/functions.html#keep-alive)
  const auto keep_alive = py::keep_alive<0, 1>();

  py::class_<Property>(m, "Property")
      .def_property_readonly("name", &Property::name)
      .def_property_readonly("description", &Property::description)
      .def_property_readonly("default_value", &Property::defaultValue)
      .def_property_readonly("valid_values", &Property::knownValues)
      .def_property_readonly("variant", py::cpp_function(&Property::variant, keep_alive))
      .def_property_readonly("auto", &Property::autoCast)
      .def("__float__", &Property::asFloat)
      .def("__int__", &Property::asInt)
      .def("__bool__", &Property::asBool)
      .def("__repr__", &Property::repr)
      .def("__str__", &Property::str);

  py::class_<PropertySet>(m, "PropertySet")
      .def("__getattr__", &PropertySet::getAttr, keep_alive)
      .def("__setattr__", &PropertySet::setAttrStr)
      .def("__setattr__", &PropertySet::setAttrBool)
      .def("__setattr__", &PropertySet::setAttrProperty)
      .def("__setattr__", &PropertySet::setAttrCast)
      .def("__dir__", &PropertySet::dir)
      .def("__repr__", &PropertySet::repr);

  py::class_<Domain, shared_ptr<Domain>>(m, "Domain")
      .def(py::init<const string&>())
      .def_property_readonly("name", &Domain::name)
      .def_property_readonly("config", py::cpp_function(&Domain::config, keep_alive))
      .def("__repr__", &Domain::repr);

  py::class_<Population, shared_ptr<Population>>(m, "Population")
      .def(py::init<const string&>())
      .def_property_readonly("name", &Population::name)
      .def_property_readonly("config", py::cpp_function(&Population::config, keep_alive))
      .def_property("size", &Population::size, &Population::setSize)
      .def("__repr__", &Population::repr);

  py::class_<Experiment, shared_ptr<Experiment>>(m, "Experiment")
      .def_property_readonly("config", py::cpp_function(&Experiment::config, keep_alive))
      .def_property_readonly("core_config",
                             py::cpp_function(&Experiment::coreConfig, keep_alive))
      .def_property_readonly("domain", &Experiment::domain)
      .def_property_readonly("population", &Experiment::population)
      .def_property_readonly("universe", &Experiment::universe)
      .def("initialize_population",
           &Experiment::initializePopulation,
           "Creates the primordial generation and prepare for evolution")
      .def("evaluate_population",
           &Experiment::evaluatePopulation,
           "Domain specific evaluation, assigning fitness values")
      .def("create_next_generation",
           &Experiment::createNextGeneration,
           "Creates the next generation, using the current fitness values")
      .def("reset",
           &Experiment::reset,
           "Resets the experiment, allowing config values to be edited")
      .def("__repr__", &Experiment::repr);

  py::class_<Universe, shared_ptr<Universe>>(m, "Universe")
      .def("new_experiment",
           &Universe::newExperiment,
           py::arg("domain"),
           py::arg("population"))
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

  m.def("create_universe",
        &createUniverse,
        py::arg("path"),
        "Creates a new Darwin universe file");

  m.def("open_universe",
        &openUniverse,
        py::arg("path"),
        "Opens an existing Darwin universe file");
}

}  // namespace darwin::python
