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

#include <third_party/sqlite/sqlite3.h>

#include <core/exception.h>
#include <tests/testcase_output.h>
#include <third_party/gtest/gtest.h>

#include <string.h>
using namespace std;

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

namespace sqlite_tests {

struct SqliteTest : public testing::Test {
  SqliteTest() {
    path_ = testdbPath();
    fs::remove(path_);

    if (sqlite3_open(path_.c_str(), &db_) != SQLITE_OK) {
      CHECK(sqlite3_close(db_) == SQLITE_OK);
      throw core::Exception("Failed to open the database");
    }
  }

  ~SqliteTest() {
    CHECK(sqlite3_close(db_) == SQLITE_OK);
    fs::remove(path_);
  }

 protected:
  static string testdbPath() {
    const auto test_info = ::testing::UnitTest::GetInstance()->current_test_info();
    return string(TEST_TEMP_PATH) + "/" + test_info->test_case_name() + "_" +
           test_info->name() + ".db";
  }

  static int execCallback(void* data, int argc, char* argv[], char* colv[]) {
    auto result = static_cast<string*>(data);
    for (int i = 0; i < argc; ++i) {
      *result += colv[i];
      *result += " = ";
      *result += (argv[i] != nullptr) ? argv[i] : "NULL";
      *result += "\n";
    }
    *result += "\n";
    return 0;
  }

  string exec(const char* statement) {
    string result;
    int rc = sqlite3_exec(db_, statement, execCallback, &result, nullptr);
    EXPECT_EQ(rc, SQLITE_OK);
    return result;
  }

 protected:
  sqlite3* db_ = nullptr;
  string path_;
};

TEST_F(SqliteTest, Smoke) {
  // just open a new database and execute an empty statement
  exec(" ");
}

TEST_F(SqliteTest, Select) {
  core_test::TestCaseOutput output;

  exec("create table foo(key text primary key, flag int default 0, value real)");
  exec("insert into foo values('a', 1, 1.0)");
  exec("insert into foo values('b', 2, 2.2)");
  exec("insert into foo values('c', 3, 3.3)");

  auto results = exec("select * from foo");
  fprintf(output, "%s", results.c_str());

  fprintf(output, "----------------------------------\n");

  auto results2 = exec("select * from sqlite_master");
  fprintf(output, "%s", results2.c_str());
}

TEST_F(SqliteTest, BasicCRUD) {
  exec("create table foo(key text primary key, flag int default 0, data blob)");
  exec("insert into foo(key) values ('blah')");
  exec("insert into foo(key, flag) values('', 1)");
  exec("insert into foo(key, flag) values(null, 1)");
  exec("insert into foo(key, flag) values(null, null)");
  exec("insert into foo(key, flag) values(' ', null)");
  exec("select * from foo");
  exec("update foo set flag = 1 where flag = 0");
  exec("select key, flag as gate from foo");
  exec("update foo set key = 'moo' where key = ''");
  exec("update foo set key = 'moo' where key = ''");
  exec("update foo set key = 'moo' where key = 'moo'");
  exec("delete from foo where key = 'moo'");
  exec("drop table foo");
}

}  // namespace sqlite_tests
