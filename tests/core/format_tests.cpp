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
#include <core/format.h>
#include <core/stringify.h>

#include <tests/testcase_output.h>
#include <third_party/gtest/gtest.h>

#include <sstream>
#include <string>
#include <vector>
using namespace std;

namespace format_tests {

TEST(FormatTest, Basic) {
  core_test::TestCaseOutput output;

  stringstream ss;
  ss << core::format("") << endl;
  ss << core::format("%s", "") << endl;
  ss << core::format("%s", string("")) << endl;
  ss << core::format("%%%%") << endl;
  ss << core::format("%% no arguments %%") << endl;
  ss << core::format("'%s'", "Hello world!") << endl;
  ss << core::format("'%s'", "~~~~ Hello world! ~~~~"s) << endl;
  ss << core::format("'%s'", string(" ... Hello world! ... ")) << endl;
  ss << core::format("prefix '%s' %%=%d sufix", "Foo"s, 123) << endl;
  ss << core::format("'%20s' + '%-20s'", string("#####"), "*****"s) << endl;
  ss << core::format("'%*s'", 20, string(" Hello world! ")) << endl;
  ss << core::format("[%+#0*.*f]", 15, 8, 32.12345f) << endl;

  fprintf(output, "%s", ss.str().c_str());
}

TEST(FormatTest, Stringify) {
  core_test::TestCaseOutput output;

  vector<int> v = { 1, 2, 3 };

  stringstream ss;
  ss << core::format("[%s]", core::toString(string("Foo"))) << endl;
  ss << core::format("[%s]", core::toString(100)) << endl;
  ss << core::format("[%s]", core::toString(v)) << endl;

  fprintf(output, "%s", ss.str().c_str());
}

}  // namespace format_tests
