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

#include <third_party/json/json.h>
using json = nlohmann::json;

#include <core/utils.h>
#include <core/matrix.h>
#include <tests/testcase_output.h>
#include <third_party/gtest/gtest.h>

#include <string.h>
#include <sstream>
using namespace std;

namespace json_tests {

// the examples from https://github.com/nlohmann/json
TEST(JsonTest, Examples) {
  core_test::TestCaseOutput output;

  // create an empty structure (null)
  json j;

  // add a number that is stored as double
  // (note the implicit conversion of j to an object)
  j["pi"] = 3.141;

  // add a Boolean that is stored as bool
  j["happy"] = true;

  // add a string that is stored as string
  j["name"] = "Niels";

  // add another null object by passing nullptr
  j["nothing"] = nullptr;

  // add an object inside the object
  j["answer"]["everything"] = 42;

  // add an array that is stored as vector (using an initializer list)
  j["list"] = { 1, 0, 2 };

  // add another object (using an initializer list of pairs)
  j["object"] = { { "currency", "USD" }, { "value", 42.99 } };

  // instead, you could also write (which looks very similar to the JSON above)
  json j2 = { { "pi", 3.141 },
              { "happy", true },
              { "name", "Niels" },
              { "nothing", nullptr },
              { "answer", { { "everything", 42 } } },
              { "list", { 1, 0, 2 } },
              { "object", { { "currency", "USD" }, { "value", "42.99" } } } };

  // a way to express the empty array []
  json empty_array_explicit = json::array();

  // ways to express the null / empty json objects
  //
  // NOTE: json({}) meaning changed between C++11 and C++17, so it's not recommended
  //
  json null_object = json();
  json empty_object = json::object();

  // a way to express an _array_ of key/value pairs
  // [["currency", "USD"], ["value", 42.99]]
  json array_not_object = json::array({ { "currency", "USD" }, { "value", 42.99 } });

  stringstream ss;
  ss << j["answer"] << endl;
  ss << j["object"]["value"] << endl;
  ss << j.dump() << endl;
  ss << j2.dump(2) << endl;
  ss << empty_array_explicit << endl;
  ss << null_object << endl;
  ss << empty_object << endl;
  ss << array_not_object << endl;

  fprintf(output, "%s", ss.str().c_str());
}

TEST(JsonTest, Values) {
  core_test::TestCaseOutput output;
  stringstream ss;

  json str = "abc 123";
  ss << str << endl;
  ss << str.dump() << endl;
  ss << str.get<string>() << endl;

  float value = 3.14f;
  json num = value;
  ss << value << " = " << num << endl;

  constexpr int kRows = 10;
  constexpr int kCols = 16;
  core::Matrix<float> m(kRows, kCols);
  for (int row = 0; row < kRows; ++row)
    for (int col = 0; col < kCols; ++col)
      m[row][col] = col + row / 10.0f;
  json matrix{
    { "rows", m.rows },
    { "cols", m.cols },
    { "values", m.values },
  };
  ss << "matrix = " << matrix << endl;

  fprintf(output, "%s", ss.str().c_str());
}

TEST(JsonTest, ForEach) {
  core_test::TestCaseOutput output;

  json j = { { "pi", 3.141 },
             { "happy", true },
             { "name", "Niels" },
             { "nothing", nullptr },
             { "answer", { { "everything", 42 } } },
             { "list", { 1, 0, 2 } },
             { "object", { { "currency", "USD" }, { "value", "42.99" } } } };

  // iterator
  for (json::iterator it = j.begin(); it != j.end(); ++it) {
    fprintf(output, "> %s\n", it->dump().c_str());
  }

  fprintf(output, "-------------------------------------\n");

  // range-based for
  for (const auto& elem : j) {
    fprintf(output, "> %s\n", elem.dump().c_str());
  }

  fprintf(output, "-------------------------------------\n");

  // iterator key/value helpers
  for (json::iterator it = j.begin(); it != j.end(); ++it) {
    fprintf(output, "'%s'='%s'\n", it.key().c_str(), it.value().dump().c_str());
  }

  fprintf(output, "-------------------------------------\n");

  // items()
  for (const auto& item : j.items()) {
    fprintf(output, "'%s'='%s'\n", item.key().c_str(), item.value().dump().c_str());
  }
}

TEST(JsonTest, Roundtrip) {
  json orig_json = { { "pi", 3.141 },
                     { "happy", true },
                     { "name", "Niels" },
                     { "nothing", nullptr },
                     { "answer", { { "everything", 42 } } },
                     { "list", { 1, 0, 2 } },
                     { "object", { { "currency", "USD" }, { "value", "42.99" } } } };

  json roundtrip_json = json::parse(orig_json.dump());

  EXPECT_EQ(orig_json, roundtrip_json);
}

TEST(JsonTest, Parse) {
  core_test::TestCaseOutput output;
  stringstream ss;

  auto conf = json::parse("{ \"happy\": true, \"pi\": 3.141 }");
  ss << conf.dump(4) << endl;

  try {
    auto bad = json::parse("{ \"happy: true, \"pi\": 3.141 }");
  } catch (const exception& e) {
    ss << "Exception: " << e.what() << endl;
  }

  fprintf(output, "%s", ss.str().c_str());
}

}  // namespace json_tests
