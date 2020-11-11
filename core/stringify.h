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
#include "io_utils.h"

#include <assert.h>
#include <initializer_list>
#include <map>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
using namespace std;

// TODO: test cases

namespace core {

template <class T>
struct TypeTag {};

//! Interface for converting values to/from strings
//! (the default implementation uses iostream I/O operators)
template <class T>
class Stringify {
 public:
  virtual ~Stringify() = default;

  //! Value to string conversion
  virtual string toString(const T& value) const = 0;
  
  //! String to value conversion
  virtual T fromString(const string& str) const = 0;
  
  //! The list of known values for type T, if available (for example enumerations)
  virtual vector<string> knownValues() const = 0;
};

//! Use iostream I/O operators for the argument type
//! \note So one way to "stringify-enable" new types is to just define these operators
template <class T>
class GenericStringify : public Stringify<T> {
 public:
  string toString(const T& value) const override {
    stringstream ss;
    ss << std::boolalpha << value;
    return ss.str();
  }

  T fromString(const string& str) const override {
    stringstream ss(str);
    T value = {};
    ss >> std::boolalpha >> value;
    if (!ss.eof()) {
      // make sure the only thing left, if any, are whitespaces
      ss >> std::ws;
    }
    if (ss.fail() || !ss.eof())
      throw Exception("invalid value string: '%s'", str.c_str());
    return value;
  }

  vector<string> knownValues() const override { return {}; }
};

//! Handles types with a fixed, known set of values (enumerations for example)
template <class T>
class StringifyKnownValues : public Stringify<T> {
 public:
  StringifyKnownValues(initializer_list<pair<T, string>> known_values) {
    for (const auto& value_str : known_values) {
      CHECK(value_to_string_.insert(value_str).second);
      CHECK(string_to_value_.insert({ value_str.second, value_str.first }).second);
    }
  }

  string toString(const T& value) const override {
    auto it = value_to_string_.find(value);
    if (it == value_to_string_.end())
      throw Exception("Unknown value");
    return it->second;
  }

  T fromString(const string& str) const override {
    auto it = string_to_value_.find(str);
    if (it == string_to_value_.end())
      throw Exception("Unknown value: '%s'", str.c_str());
    return it->second;
  }

  vector<string> knownValues() const override {
    vector<string> values;
    for (const auto& kv : string_to_value_)
      values.push_back(kv.first);
    return values;
  }

 private:
  unordered_map<T, string> value_to_string_;
  map<string, T> string_to_value_;
};

//! Maps strings unmodified
//! (going through I/O operators doesn't handle strings with embedded whitespace)
class StringifyStrings : public Stringify<string> {
 public:
  string toString(const string& value) const override { return value; }

  string fromString(const string& str) const override { return str; }

  vector<string> knownValues() const override { return {}; }
};

// the default "catch all" using the GenericStringify<T>
template <class T>
auto customStringify(core::TypeTag<T>) {
  static auto stringify = new GenericStringify<T>();
  return stringify;
}

inline auto customStringify(core::TypeTag<bool>) {
  static auto stringify = new StringifyKnownValues<bool>{
    { true, "true" },
    { false, "false" },
  };
  return stringify;
}

inline auto customStringify(core::TypeTag<string>) {
  static auto stringify = new StringifyStrings();
  return stringify;
}

//! Returns the stringifier for type T
template <class T>
const Stringify<T>* stringify() {
  return customStringify(TypeTag<T>{});
}

//! Convenience helper to stringify a value
template <class T>
string toString(const T& value) {
  return stringify<T>()->toString(value);
}

//! Convenience helper to parse a string value
template <class T>
T fromString(const string& str) {
  return stringify<T>()->fromString(str);
}

//! Convenience helper to return the set of known values
template <class T>
vector<string> knownValues() {
  return stringify<T>()->knownValues();
}

}  // namespace core
