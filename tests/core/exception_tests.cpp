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
#include <core/exception.h>

#include <third_party/gtest/gtest.h>

namespace exception_tests {

TEST(ExceptionTest, Basic) {
  bool caught = false;
  try {
    throw core::Exception("Message");
  } catch (const core::Exception& e) {
    EXPECT_STREQ(e.what(), "Message");
    caught = true;
  }
  EXPECT_TRUE(caught);
}

TEST(ExceptionTest, Empty) {
  bool caught = false;
  try {
    throw core::Exception("");
  } catch (const core::Exception& e) {
    EXPECT_STREQ(e.what(), "");
    caught = true;
  }
  EXPECT_TRUE(caught);
}

TEST(ExceptionTest, Format) {
  bool caught = false;
  try {
    try {
      throw core::Exception("[%s=%d]", "Test", 123);
    } catch (std::exception&) {
      // make things more interesting, rethrow...
      throw;
    } catch (...) {
      // nop
    }
  } catch (const core::Exception& e) {
    EXPECT_STREQ(e.what(), "[Test=123]");
    caught = true;
  }
  EXPECT_TRUE(caught);
}

}  // namespace exception_tests
