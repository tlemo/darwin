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
#include "exception.h"
#include "stringify.h"

#include <third_party/json/json.h>
using nlohmann::json;

#include <assert.h>
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>
using namespace std;

namespace core {

class PropertySet;

template <class T>
class TypedProperty;

//! Reflection interface to a property in a PropertySet
//! \sa PropertySet
class Property {
 public:
  Property(PropertySet* parent, const char* name, const char* description)
      : parent_(parent), name_(name), description_(description) {
    CHECK(parent != nullptr);
    CHECK(!name_.empty());
    CHECK(!description_.empty());
  }

  virtual ~Property() = default;

  //! Property name
  const string& name() const { return name_; }
  
  //! Property description
  const string& description() const { return description_; }
  
  //! The PropertySet which contains this property
  PropertySet* parent() const { return parent_; }

  //! The property value, formatted as a string
  virtual string value() const = 0;
  
  //! The default property value, formatted as a string
  virtual string defaultValue() const = 0;
  
  //! Update the current value from a string representation
  virtual void setValue(const string& str) = 0;
  
  //! The list of known possible values (when available, for example enums)
  virtual vector<string> knownValues() const = 0;

  //! Copies values between properties of the same runtime type
  //! \throws std::bad_cast if the types don't match
  virtual void copyFrom(const Property& src) = 0;

  //! Type-safe accessor to the native property value
  //! \throws core::Exception if the runtime type doesn't match
  template <class T>
  const T& nativeValue() const {
    auto typed_property = dynamic_cast<const TypedProperty<T>*>(this);
    if (typed_property == nullptr)
      throw core::Exception("Invalid property type");
    return typed_property->nativeValue();
  }

 private:
  PropertySet* parent_ = nullptr;
  const string name_;
  const string description_;
};

template <class T>
class TypedProperty : public Property {
 public:
  TypedProperty(PropertySet* parent,
                const char* name,
                T default_value,
                T* value,
                const char* description)
      : Property(parent, name, description),
        default_value_(default_value),
        value_(value) {}

  string value() const override { return core::toString(*value_); }

  string defaultValue() const override { return core::toString(default_value_); }

  void setValue(const string& str) override;

  vector<string> knownValues() const override { return core::knownValues<T>(); }

  void copyFrom(const Property& src) override;

  const T& nativeValue() const { return *value_; }

 private:
  T default_value_;
  T* value_ = nullptr;
};

//! The foundation for data structures supporting runtime reflection
//! 
//! PropertySet provides portable-C++ support for structures containing self-describing
//! properties (no build time pre-processing or external tools are required, this is
//! a compiler-only solution)
//! 
//! - Each property has a **name**, **type**, **default value** and
//!     a brief **description string**
//! - Properties map directly to a C++ struct data member with the specified type
//! - Each instance of PropertySet-derived structs includes its own reflection metadata
//! 
//! ### Usage
//! 
//! In order to support the runtime reflection, a structure must derive from 
//! core::Properties. The individual properties are then declared using the PROPERTY()
//! macro, for example:
//!
//! ```cpp
//! // PROPERTY(name, type, default_value, description)
//! 
//! struct Config : public core::PropertySet {
//!   PROPERTY(max_value, int, 100, "Maximum value");
//!   PROPERTY(resolution, float, 0.3f, "Display resolution");
//!   PROPERTY(name, string, "darwin", "Name");
//!   PROPERTY(layers, vector<int>, {}, "Hidden layer sizes");
//! };
//! ```
//! 
//! ### Direct member access (as regular struct members)
//! 
//! This zero-overhead compared with regular C++ structs was one of the key requirements,
//! since the reflected structures are commonly configuration values which could be
//! read on the hot paths (in contrast, the high PropertySet instantiation cost and the
//! reflection path overhead were deemed acceptable)
//! 
//! ```cpp
//! Config config;
//! 
//! // set Config::max_value
//! config.max_value = 75;
//! 
//! // read Config::name
//! auto name = config.name;
//! ```
//! 
//! ### Using the runtime reflection support
//! 
//! ```cpp
//! void printProperties(const core::PropertySet* config) {
//!   // enumerate properties
//!   for (const auto& property : config->properties()) {
//!     // read the property name and value as strings
//!     core::log("%s = %s\n", property->name(), property->value());
//!   }
//! }
//! ```
//! 
//! \note This is a specialized reflection solution intended to support dynamic UIs and
//!   serializing configuration values. Since every instance of PropertySet-derived
//!   structures builds its own internal reflection metadata it may not be appropriate for
//!   use cases where there are lots of instances of the reflected structures, or when 
//!   the object creation performance is critical.
//! 
//! \sa Property
//! 
//! \todo Property constraints/validation (range, resolution, ...)
//! \todo Formatting options?
//! 
class PropertySet : public core::NonCopyable {
 public:
  PropertySet() = default;
  virtual ~PropertySet() = default;

  //! Accessor to the list of properties
  vector<Property*> properties() {
    vector<Property*> properties;
    for (const auto& property : properties_)
      properties.push_back(property.get());
    return properties;
  }

  //! Accessor to the list of properties
  vector<const Property*> properties() const {
    vector<const Property*> properties;
    for (const auto& property : properties_)
      properties.push_back(property.get());
    return properties;
  }

  //! Resets all the properties to the default values
  void resetToDefaultValues() {
    for (const auto& property : properties_)
      property->setValue(property->defaultValue());
  }

  //! Support for transfering values between property sets
  //! \throws core::Exception, if the dynamic types are not compatible
  void copyFrom(const PropertySet& src) {
    CHECK(typeid(*this) == typeid(src));
    CHECK(properties_.size() == src.properties_.size());
    for (size_t i = 0; i < properties_.size(); ++i)
      properties_[i]->copyFrom(*src.properties_[i]);
  }

  //! After sealing a PropertySet, all attempts to modify it through
  //! the Property interface will throw an exception (even if setting to the same value)
  void seal() {
    CHECK(!sealed_);
    sealed_ = true;
  }

  // CONSIDER: is setting a property to the same value really a modification?
  void propertyChanged() {
    if (sealed_)
      throw core::Exception("Attempting to modify a sealed PropertySet");
  }

  //! Serialize all the properties to a json object
  //!
  //! \note We use the string representation of values, even when there's
  //!   a native json representation, for example integers.
  //!
  json toJson() const {
    json json_obj = json::object();
    for (const auto& property : properties_)
      json_obj[property->name()] = property->value();
    return json_obj;
  }

  //! Deserialize a set of properties from a json object
  //! 
  //! The deserialization is not strict:
  //!  - Extra json properties are ignored
  //!  - Missing properties will be set to the default values
  //!
  void fromJson(const json& json_obj) {
    CHECK(json_obj.is_object());

    bool at_least_one_value = false;
    for (const auto& property : properties_) {
      auto json_it = json_obj.find(property->name());
      if (json_it != json_obj.end()) {
        property->setValue(json_it.value());
        at_least_one_value = true;
      } else
        property->setValue(property->defaultValue());
    }

    // sanity check, if none of the fields match we have something
    // completly different than we expected
    CHECK(json_obj.empty() || properties_.empty() || at_least_one_value);
  }

 protected:
  template <class T>
  T registerProperty(const char* name,
                     T default_value,
                     T* value,
                     const char* description) {
    assert(name != nullptr);
    assert(value != nullptr);
    properties_.push_back(
        make_unique<TypedProperty<T>>(this, name, default_value, value, description));
    return default_value;
  }

 private:
  vector<unique_ptr<Property>> properties_;
  bool sealed_ = false;
};

template <class T>
void TypedProperty<T>::setValue(const string& str) {
  *value_ = core::fromString<T>(str);
  parent()->propertyChanged();
}

template <class T>
void TypedProperty<T>::copyFrom(const Property& src) {
  *value_ = *dynamic_cast<const TypedProperty&>(src).value_;
  parent()->propertyChanged();
}

// convenience macro for defining properties
#define PROPERTY(name, type, default_value, description) \
  type name { registerProperty<type>(#name, type(default_value), &name, (description)) }

}  // namespace core
