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

#include "exception.h"

#include <iostream>
#include <vector>
using namespace std;

namespace core {

// extracts an exact match of the specified token, after skipping any leading whitespaces
// (it throws if the input does not match the token)
inline istream& operator>>(istream& stream, const char* token) {
  istream::sentry sentry(stream);

  for (const char* p = token; *p; ++p) {
    if (stream.fail())
      throw core::Exception("bad stream state");
    if (stream.get() != *p)
      throw core::Exception("input doesn't match the expected token '%s'", token);
  }

  return stream;
}

// peek at the first non-whitespace character, without extracting it
inline char nextSymbol(istream& stream) {
  if (stream.fail())
    throw core::Exception("bad stream state");
  return char((stream >> std::ws).peek());
}

template <class T>
ostream& operator<<(ostream& stream, const vector<T>& v) {
  stream << "{ ";
  for (size_t i = 0; i < v.size(); ++i) {
    if (i != 0)
      stream << ", ";
    stream << v[i];
  }
  stream << " }";
  return stream;
}

template <class T>
istream& operator>>(istream& stream, vector<T>& v) {
  vector<T> extracted_vector;
  stream >> "{";

  if (nextSymbol(stream) != '}') {
    for (;;) {
      T value = {};
      stream >> value;
      extracted_vector.push_back(value);

      if (nextSymbol(stream) == '}')
        break;

      stream >> ",";
    }
  }

  stream >> "}";
  v = extracted_vector;
  return stream;
}

}  // namespace core
