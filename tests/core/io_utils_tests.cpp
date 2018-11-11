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
#include <core/io_utils.h>
#include <tests/testcase_output.h>
#include <third_party/gtest/gtest.h>

#include <stdio.h>
#include <sstream>
#include <vector>
using namespace std;

namespace io_utils_tests {

using core::operator>>;
using core::operator<<;

TEST(IoUtilsTest, Roundtrip) {
  stringstream ss;

  const vector<bool> no_values = {};
  const vector<char> char_values = { '*' };
  const vector<int> int_values = { -1, 0, 1, 100 };
  const vector<double> double_values = { -100.001, -1, -0.1, 0, 0.01, 1, 1.1 };

  ss << "Testing: " << no_values << " & " << char_values << "&" << int_values << " & "
     << double_values << ".";

  vector<bool> no_values_ex;
  vector<char> char_values_ex;
  vector<int> int_values_ex;
  vector<double> double_values_ex;

  ss >> "Testing:" >> no_values_ex >> "&" >> char_values_ex >> "&" >> int_values_ex >>
      "&" >> double_values_ex >> ".";

  EXPECT_EQ(no_values, no_values_ex);
  EXPECT_EQ(char_values, char_values_ex);
  EXPECT_EQ(int_values, int_values_ex);
  EXPECT_EQ(double_values, double_values_ex);
}

TEST(IoUtilsTest, Reading) {
  stringstream ss("Testing $_$   123.abc   Foo ");
  string str;

  ss >> "Testing";
  EXPECT_EQ(core::nextSymbol(ss), '$');
  ss >> "$_$ ";
  EXPECT_EQ(core::nextSymbol(ss), '1');
  ss >> "123";
  EXPECT_EQ(core::nextSymbol(ss), '.');
  ss >> str;
  EXPECT_EQ(str, ".abc");
  EXPECT_EQ(core::nextSymbol(ss), 'F');
  ss >> str;
  EXPECT_EQ(str, "Foo");
  EXPECT_TRUE(!ss.fail());
  EXPECT_EQ(core::nextSymbol(ss), iostream::traits_type::eof());
}

TEST(IoUtilsTest, Writing) {
  stringstream ss;

  vector<char> char_values = { 'x' };
  vector<int> int_values = { -1, 0, 1, 100 };
  vector<double> double_values = { -100.001, -1, -0.1, 0, 0.01, 1, 1.1 };
  vector<const char*> strings = { "a", "Bb", "Cc cC", " dDDd " };

  ss << "[Testing...]" << std::endl;
  ss << vector<bool>() << std::endl;
  ss << char_values << std::endl;
  ss << int_values << std::endl;
  ss << double_values << std::endl;
  ss << strings << std::endl;
  ss << "[Done]" << std::endl;

  core_test::TestCaseOutput output;
  fprintf(output, "%s", ss.str().c_str());
}

}  // namespace io_utils_tests
