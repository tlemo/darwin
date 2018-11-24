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
  
  //! Return the optional child PropertySet, or `nullptr`
  virtual PropertySet* childPropertySet() { return nullptr; }
  
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

//! A variant type (tagged-union) with PropertySet fields
//! 
//! The PropertySetVariant offers a simple solution for grouping a set of mutually
//! exclusive property sets. The tag type (normally an enum) tracks which field (case)
//! is active.
//! 
//! ### Defining variants
//! 
//! First, we need a scalar tag type:
//! 
//! ```cpp
//! enum class VariantTag {
//!   Empty,
//!   Basic,
//!   Extra,
//! };
//! 
//! // the tag type must support core::toString() and core::fromString()
//! inline auto customStringify(core::TypeTag<VariantTag>) {
//!   static auto stringify = new core::StringifyKnownValues<VariantTag>{
//!     { VariantTag::Empty, "empty" },
//!     { VariantTag::Basic, "basic" },
//!     { VariantTag::Extra, "extra" },
//!   };
//!   return stringify;
//! }
//! ```
//! 
//! The variant type must derive from PropertySetVariant, and the `CASE(tag, name, type)`
//! macro is used to declare the variant fields (cases):
//! 
//! ```cpp
//! struct VariantType : public core::PropertySetVariant<VariantTag> {
//!   CASE(VariantTag::Empty, empty, EmptyProperties);
//!   CASE(VariantTag::Basic, basic, BasicProperties);
//!   CASE(VariantTag::Extra, extra, ExtraProperties);
//! };
//! ```
//! 
//! Finally, the variant type is intended to be used inside a PropertySet, using
//! the `VARIANT(name, type, default_case, description)` macro to declare variant fields.
//! 
//! ```cpp
//! struct Properties : public core::PropertySet {
//!   ...
//!   VARIANT(, VariantType, VariantTag::Basic, "A variant property");
//!   ...
//! };
//! ```
//! 
//! \sa PropertySet
//! 
template <class TAG>
class PropertySetVariant : public core::NonCopyable {
 public:
  using TagType = TAG;
  
 public:
  virtual ~PropertySetVariant() = default;

  //! The tag value indicating the active PropertySet case
  TAG tag() const { return tag_; }

  //! Selects the active PropertySet case
  void selectCase(TAG tag) {
    if (cases_.find(tag_) == cases_.end())
      throw core::Exception("Invalid variant case");
    tag_ = tag;
  }
  
  //! Returns the active PropertySet
  const PropertySet* activeCase() const {
    auto it = cases_.find(tag_);
    CHECK(it != cases_.end());
    return it->second;
  }

  //! Returns the active PropertySet
  PropertySet* activeCase() {
    auto it = cases_.find(tag_);
    CHECK(it != cases_.end());
    return it->second;
  }

  //! Serialize to a json object
  //! 
  //! \note All the PropertySet cases are serialized, not just the active one
  //!
  json toJson() const {
    json json_obj = json::object();
    json_obj["tag"] = core::toString(tag_);
    for (const auto& [case_tag, case_value] : cases_) {
      json_obj[core::toString(case_tag)] = case_value->toJson();
    }
    return json_obj;
  }

  //! Deserialize from a json object
  //! 
  //! The deserialization is not strict:
  //!  - Extra cases are ignored
  //!  - Missing cases will be set to the default values
  //!
  void fromJson(const json& json_obj) {
    CHECK(json_obj.is_object());

    bool at_least_one_case = false;
    tag_ = core::fromString<TAG>(json_obj.at("tag"));
    for (auto& [case_tag, case_value] : cases_) {
      auto json_it = json_obj.find(core::toString(case_tag));
      if (json_it != json_obj.end()) {
        case_value->fromJson(json_it.value());
        at_least_one_case = true;
      } else {
        case_value->resetToDefaultValues();
      }
    }

    // sanity check
    CHECK(cases_.empty() || at_least_one_case);
  }

  //! Transfer values between two property set variants
  void copyFrom(const PropertySetVariant& src) {
    CHECK(typeid(*this) == typeid(src));
    CHECK(cases_.size() == src.cases_.size());
    tag_ = src.tag_;
    for (const auto& [case_tag, case_value] : src.cases_) {
      cases_.at(case_tag)->copyFrom(*case_value);
    }
  }

 protected:
  bool registerCase(TAG tag, PropertySet* property_set) {
    CHECK(cases_.insert({ tag, property_set }).second, "Duplicate cases");
    tag_ = tag;
    return true;
  }

 private:
  TAG tag_ = TAG(-1);
  map<TAG, PropertySet*> cases_;
};

template <class T>
class VariantProperty : public Property {
  using TagType = typename T::TagType;

 public:
  VariantProperty(PropertySet* parent,
                  const char* name,
                  TagType default_case,
                  T* variant,
                  const char* description)
      : Property(parent, name, description),
        default_case_(default_case),
        variant_(variant) {}

  string value() const override { return core::toString(variant_->tag()); }

  string defaultValue() const override { return core::toString(default_case_); }

  PropertySet* childPropertySet() override { return variant_->activeCase(); }

  void setValue(const string& str) override;

  vector<string> knownValues() const override { return core::knownValues<TagType>(); }

  void copyFrom(const Property& src) override;

 private:
  TagType default_case_;
  T* variant_ = nullptr;
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

  //! Transfer values between two property sets
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
    // completely different than we expected
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

  template <class V, class TAG>
  bool registerVariant(const char* name,
                       TAG default_case,
                       V* variant,
                       const char* description) {
    assert(name != nullptr);
    assert(variant != nullptr);
    variant->selectCase(default_case);
    properties_.push_back(
        make_unique<VariantProperty<V>>(this, name, default_case, variant, description));
    return true;
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

template <class T>
void VariantProperty<T>::setValue(const string& str) {
  variant_->selectCase(core::fromString<TagType>(str));
  parent()->propertyChanged();
}

template <class T>
void VariantProperty<T>::copyFrom(const Property& src) {
  variant_->copyFrom(*dynamic_cast<const VariantProperty&>(src).variant_);
  parent()->propertyChanged();
}

// convenience macro for defining properties
#define PROPERTY(name, type, default_value, description) \
  type name { registerProperty<type>(#name, type(default_value), &name, (description)) }

// convenience macro for defining variants
#define VARIANT(name, type, default_case, description) \
  type name;                                           \
  bool name##_INIT { registerVariant<type>(#name, (default_case), &name, (description)) }

// convenience macro for defining variant cases
#define CASE(tag, name, type) \
  type name;                  \
  bool name##_INIT { registerCase((tag), &name) }

}  // namespace core
