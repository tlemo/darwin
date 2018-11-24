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

#include <core/utils.h>
#include <core/properties.h>
#include <tests/testcase_output.h>
#include <third_party/gtest/gtest.h>

#include <third_party/json/json.h>
using json = nlohmann::json;

#include <stdio.h>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

namespace properties_tests {

using core::operator>>;
using core::operator<<;

// enumeration
enum class Operator : int {
  Add,
  Subtract,
  Multiply,
  Divide,
};

auto customStringify(core::TypeTag<Operator>) {
  static auto stringify = new core::StringifyKnownValues<Operator>{
    { Operator::Add, "add" },
    { Operator::Subtract, "subtract" },
    { Operator::Multiply, "multiply" },
    { Operator::Divide, "divide" },
  };
  return stringify;
}

// using a custom property type
struct CustomType {
  float x = 0;
  float y = 0;

  // stringify support
  friend ostream& operator<<(ostream& stream, const CustomType& ct) {
    return stream << "{ " << ct.x << ", " << ct.y << " }";
  }

  friend istream& operator>>(istream& stream, CustomType& ct) {
    return stream >> "{" >> ct.x >> "," >> ct.y >> "}";
  }

  bool operator==(const CustomType& other) const { return x == other.x && y == other.y; }

  bool operator!=(const CustomType& other) const { return !operator==(other); }
};

// various helpers for using initializer_list default values
const vector<double> kDefaultValues = { -1, 0, 0.1, 0.2, 1.001 };

#define DEFAULT_VALUES \
  { 0, 10, 20, 30 }

#define INIT_LIST(...) \
  { __VA_ARGS__ }

// our test structure
struct TestProperties : public core::PropertySet {
  PROPERTY(max_value, int, 100, "Maximum value");
  PROPERTY(resolution, float, 0.3f, "Display resolution");
  PROPERTY(name, string, " <\"El chupacabra\"> ", "Name");
  PROPERTY(flag, bool, false, "A boolean flag");

  PROPERTY(acc_operator, Operator, Operator::Add, "Enumeration");

  PROPERTY(layers, vector<int>, {}, "Hidden layer sizes");

  PROPERTY(values, vector<double>, kDefaultValues, "Vector of real values");

  PROPERTY(more_values, vector<double>, INIT_LIST(1, 2, 3), "More real values ...");

  PROPERTY(extra_values, vector<double>, DEFAULT_VALUES, "Even more real values ...");

  PROPERTY(values_init,
           vector<double>,
           vector<double>({ -1, -2, -3 }),
           "Almost natural use of initializer lists");

  PROPERTY(custom, CustomType, {}, "A simple custom type");

  PROPERTY(custom_init,
           CustomType,
           CustomType({ 1.0f, -100.0f }),
           "Initialized custom type");
};

// make sure we handle empty structs
struct EmptyProperties : public core::PropertySet {};

TEST(PropertiesTest, Const) {
  core_test::TestCaseOutput output;

  const TestProperties test;

  for (const auto& property : test.properties()) {
    fprintf(output,
            "%s = '%s' ['%s'] # %s\n",
            property->name().c_str(),
            property->value().c_str(),
            property->defaultValue().c_str(),
            property->description().c_str());
  }
}

TEST(PropertiesTest, Enumerate) {
  core_test::TestCaseOutput output;

  TestProperties test;

  for (const auto& property : test.properties()) {
    fprintf(output,
            "name: '%s'\ndescription: '%s'\n\n",
            property->name().c_str(),
            property->description().c_str());
  }
}

TEST(PropertiesTest, SetValuesDirect) {
  core_test::TestCaseOutput output;

  TestProperties test;

  test.max_value = 5;
  test.resolution = 10.5f;
  test.name = " *** lemo 123 *** ";
  test.flag = true;
  test.acc_operator = Operator::Multiply;
  test.layers = { 16, 8, 4 };
  test.more_values = {};
  test.custom.x = 64;
  test.custom.y = 16;

  for (const auto& property : test.properties()) {
    fprintf(output, "%s = '%s'\n", property->name().c_str(), property->value().c_str());
  }

  fprintf(output, "\n-------------------------------\n");

  test.resetToDefaultValues();

  for (const auto& property : test.properties()) {
    fprintf(output, "%s = '%s'\n", property->name().c_str(), property->value().c_str());
  }
}

TEST(PropertiesTest, SetValuesString) {
  core_test::TestCaseOutput output;

  TestProperties test;

  // build an prop_name -> prop index
  map<string, core::Property*> index;
  for (const auto& property : test.properties())
    CHECK(index.insert({ property->name(), property }).second);

  index["max_value"]->setValue("5");
  index["resolution"]->setValue(" 10.5 ");
  index["name"]->setValue(" -- Lemo 123 -- ");
  index["flag"]->setValue("true");
  index["custom"]->setValue(" {64,16} ");
  index["acc_operator"]->setValue("multiply");

  index["layers"]->setValue("{ 16, 8, 4 }");
  index["values"]->setValue("{}");
  index["more_values"]->setValue("{-1.1,0,+123.456}");
  index["extra_values"]->setValue(" { 0.0 , 0.1, 0.2 } ");
  index["values_init"]->setValue("{ 3.14 }");

  EXPECT_THROW(index["values"]->setValue(""), core::Exception);
  EXPECT_THROW(index["values"]->setValue("{1,b}"), core::Exception);
  EXPECT_THROW(index["values"]->setValue("{1,2},"), core::Exception);
  EXPECT_THROW(index["values"]->setValue("{1,2}x"), core::Exception);
  EXPECT_THROW(index["values"]->setValue("[1,2]"), core::Exception);

  EXPECT_THROW(index["more_values"]->setValue(" { "), core::Exception);
  EXPECT_THROW(index["more_values"]->setValue("{3.14"), core::Exception);
  EXPECT_THROW(index["more_values"]->setValue("{3.14,"), core::Exception);
  EXPECT_THROW(index["more_values"]->setValue("{,3.14 }"), core::Exception);

  EXPECT_THROW(index["custom"]->setValue("64,16"), core::Exception);
  EXPECT_THROW(index["custom"]->setValue("  "), core::Exception);
  EXPECT_THROW(index["custom"]->setValue("{}"), core::Exception);

  EXPECT_THROW(index["acc_operator"]->setValue("Sqrt"), core::Exception);

  EXPECT_THROW(index["max_value"]->setValue("6.5"), core::Exception);
  EXPECT_THROW(index["max_value"]->setValue(""), core::Exception);
  EXPECT_THROW(index["max_value"]->setValue("   "), core::Exception);
  EXPECT_THROW(index["max_value"]->setValue("x32"), core::Exception);

  for (const auto& property : test.properties()) {
    fprintf(output, "%s = '%s'\n", property->name().c_str(), property->value().c_str());
  }
}

TEST(PropertiesTest, EmptyProperties) {
  core_test::TestCaseOutput output;

  EmptyProperties empty;

  auto json_obj = empty.toJson();
  empty.fromJson(json_obj);

  fprintf(output, "%s", json_obj.dump(2).c_str());
}

TEST(PropertiesTest, JsonRoundtrip) {
  TestProperties source;
  source.max_value = 5;
  source.resolution = 10.5f;
  source.name = " -~= \"BLOL\" =~- ";
  source.flag = true;
  source.acc_operator = Operator::Multiply;
  source.layers = { 16, 8, 4 };
  source.more_values = {};
  source.extra_values = { -1, 0, 1 };
  source.custom.x = 64;
  source.custom.y = 16;
  string json_str = source.toJson().dump(2);

  TestProperties copy;
  copy.fromJson(json::parse(json_str));

  // compare the serialized json objects
  EXPECT_EQ(source.toJson(), copy.toJson());

  // compare the PropertySets pairwise
  auto source_properties = source.properties();
  auto copy_properties = copy.properties();
  ASSERT_EQ(source_properties.size(), copy_properties.size());
  for (size_t i = 0; i < source_properties.size(); ++i) {
    ASSERT_EQ(source_properties[i]->name(), copy_properties[i]->name());
    EXPECT_EQ(source_properties[i]->value(), copy_properties[i]->value());
  }
}

TEST(PropertiesTest, ToJson) {
  core_test::TestCaseOutput output;

  TestProperties test;

  auto json_obj = test.toJson();
  auto json_str = json_obj.dump(2);
  fprintf(output, "%s", json_str.c_str());
}

TEST(PropertiesTest, FromJson) {
  core_test::TestCaseOutput output;

  const string json_str = R"(
    {
      "max_value": "256",
      "resolution": "0.05",
      "name": " <\"El chupacabra\"> ",
      "layers": "{ 10, 20, 30 }",
      "values": "{ }",
      
      "acc_operator_ex": "multiply",
      "flag_ex": "true",
      "acc_operator_ex": "add",
      "custom_ex": "{ 1000, 2000 }"
    })";

  TestProperties test;

  test.fromJson(json::parse(json_str));

  for (const auto& property : test.properties()) {
    fprintf(output, "%s = '%s'\n", property->name().c_str(), property->value().c_str());
  }
}

TEST(PropertiesTest, Seal) {
  TestProperties test;

  // change ok
  for (core::Property* property : test.properties())
    property->setValue(property->defaultValue());

  test.seal();

  // not allowed to change after sealing it
  // (even attempting to set it to the same value)
  for (core::Property* property : test.properties()) {
    EXPECT_THROW(property->setValue(property->defaultValue()), core::Exception);
    EXPECT_THROW(property->copyFrom(*property), core::Exception);
  }
}

TEST(PropertiesTest, DefaultValues) {
  core_test::TestCaseOutput output;

  TestProperties test;

  for (const auto& property : test.properties()) {
    fprintf(output,
            "%s = '%s'\n",
            property->name().c_str(),
            property->defaultValue().c_str());
  }
}

TEST(PropertiesTest, KnownValues) {
  core_test::TestCaseOutput output;

  TestProperties test;

  for (const auto& property : test.properties()) {
    fprintf(output, "%s:", property->name().c_str());
    for (const auto& known_value : property->knownValues())
      fprintf(output, " '%s'", known_value.c_str());
    fprintf(output, "\n");
  }
}

}  // namespace properties_tests
