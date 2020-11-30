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
#include <core/database.h>
#include <core/exception.h>

#include <tests/testcase_output.h>
#include <third_party/gtest/gtest.h>

#include <inttypes.h>
#include <memory>
#include <string>
#include <tuple>
using namespace std;

#include <filesystem>
namespace fs = std::filesystem;

namespace database_tests {

struct DatabaseTest : public testing::Test {
  DatabaseTest() {
    path = testdbPath();
    fs::remove(path);

    db = make_unique<db::Connection>(path, db::OpenMode::CreateNew);
  }

  ~DatabaseTest() {
    db.reset();
    fs::remove(path);
  }

 protected:
  static string testdbPath() {
    const auto test_info = ::testing::UnitTest::GetInstance()->current_test_info();
    return string(TEST_TEMP_PATH) + "/" + test_info->test_case_name() + "_" +
           test_info->name() + ".db";
  }

 protected:
  unique_ptr<db::Connection> db;
  string path;
};

TEST_F(DatabaseTest, Smoke) {
  db->exec("   select * from sqlite_master   ");
}

TEST_F(DatabaseTest, ParameterBinding) {
  core_test::TestCaseOutput output;

  db->exec("create table params(id integer primary key, value real, name text)");

  db->exec("insert into params(id, value, name) values(?, ?, ?)", 1, 3.14, "Pi");
  db->exec("insert into params(name, value) values(?, ?)", "Max", 100);
  db->exec("insert into params values(?, ?, ?)", nullopt, nullopt, nullopt);
  db->exec("insert into params values(?, ?, ?)", nullopt, 0.1, "Haha let's see");

  // too many arguments
  EXPECT_THROW(db->exec("insert into params(value) values(?)", 5, 7), core::Exception);

  auto results =
      db->exec<int, double, string>("select id, value, name from params order by id");

  for (const auto& row : results) {
    fprintf(output,
            "%d, %f, '%s'\n",
            get<0>(row).value_or(0),
            get<1>(row).value_or(0),
            get<2>(row).value_or("null").c_str());
  }
}

TEST_F(DatabaseTest, ManualTransactions) {
  core_test::TestCaseOutput output;

  db->beginTransaction();

  // already in a transaction
  EXPECT_THROW(db->beginTransaction(), core::Exception);

  db->exec("create table transact_1(id integer primary key, value real, name text)");
  db->exec("insert into transact_1(value, name) values(3.14, 'PI')");

  db->commit();

  // not in a transaction
  EXPECT_THROW(db->commit(), core::Exception);

  db->beginTransaction(db::TransactionOption::Immediate);
  db->exec("insert into transact_1(value, name) values(null, 'nothing')");
  db->exec("insert into transact_1(value, name) values(10, null)");
  db->exec("insert into transact_1(value, name) values(null, null)");
  db->rollback();

  db->beginTransaction(db::TransactionOption::Exclusive);
  db->exec("insert into transact_1(value, name) values(null, null)");
  db->rollback();

  for (const auto& [id, value, name] : db->exec<int, double, string>(
           "select id, value, name from transact_1 order by id")) {
    fprintf(output,
            "%d, %f, '%s'\n",
            id.value_or(0),
            value.value_or(0),
            name.value_or("null").c_str());
  }
}

TEST_F(DatabaseTest, ScopeTransactions) {
  core_test::TestCaseOutput output;

  db::TransactionScope create_transaction(*db);

  db->exec("create table transact_1(id integer primary key, value real, name text)");
  db->exec("insert into transact_1(value, name) values(3.14, 'PI')");

  create_transaction.commit();

  try {
    db::TransactionScope insert_transaction(*db, db::TransactionOption::Exclusive);
    db->exec("insert into transact_1(value, name) values(null, 'nothing')");
    db->exec("insert into transact_1(value, name) values(10, null)");
    throw core::Exception("mid-transaction exception");
    db->exec("insert into transact_1(value, name) values(null, null)");
    insert_transaction.commit();
  } catch (const exception& e) {
    fprintf(output, "caught exception: %s\n", e.what());
  }

  for (const auto& [id, value, name] : db->exec<int, double, string>(
           "select id, value, name from transact_1 order by id")) {
    fprintf(output,
            "%d, %f, '%s'\n",
            id.value_or(0),
            value.value_or(0),
            name.value_or("null").c_str());
  }
}

TEST_F(DatabaseTest, Inserts) {
  core_test::TestCaseOutput output;

  db->exec("create table inserts(id integer primary key, value real, name text)");

  fprintf(output, "before/rowid: %" PRId64 "\n", db->lastInsertRowId());

  db->exec("insert into inserts(value, name) values(3.14, 'PI')");
  fprintf(output, "rowid: %" PRId64 "\n", db->lastInsertRowId());

  db->exec("insert into inserts(value, name) values(null, 'nothing')");
  fprintf(output, "rowid: %" PRId64 "\n", db->lastInsertRowId());

  db->exec("insert into inserts(value, name) values(10, null)");
  fprintf(output, "rowid: %" PRId64 "\n", db->lastInsertRowId());

  db->exec("insert into inserts(value, name) values(null, null)");
  fprintf(output, "rowid: %" PRId64 "\n", db->lastInsertRowId());

  fprintf(output, "after/rowid: %" PRId64 "\n", db->lastInsertRowId());
}

TEST_F(DatabaseTest, ResultSet) {
  core_test::TestCaseOutput output;

  db->exec("create table results(id integer primary key, value real, name text)");

  db->exec("insert into results(value, name) values(3.14, 'PI')");
  db->exec("insert into results(value, name) values(null, 'nothing')");
  db->exec("insert into results(value, name) values(10, null)");
  db->exec("insert into results(value, name) values(null, null)");

  // not enough result columns
  EXPECT_THROW(db->exec<double>("select value, name from results"), core::Exception);

  // too many result columns
  EXPECT_THROW((db->exec<double, string, int>("select value, name from results")),
               core::Exception);

  // result types don't match the query results
  EXPECT_THROW((db->exec<string, double>("select value, name from results")),
               core::Exception);

  auto results =
      db->exec<int, double, string>("select id, value, name from results order by id");

  for (const auto& [id, value, name] : results) {
    fprintf(output,
            "%d, %f, '%s'\n",
            id.value_or(0),
            value.value_or(0),
            name.value_or("null").c_str());
  }
}

TEST_F(DatabaseTest, Pragmas) {
  // PRAGMA integrity_check
  {
    auto results = db->exec<string>("pragma integrity_check");
    EXPECT_EQ(results.singleValue(), "ok");
  }

  // PRAGMA schema_version
  {
    auto results = db->exec<int>("pragma schema_version");
    EXPECT_EQ(results.singleValue(), 0);
  }

  // PRAGMA application_id
  {
    db->exec("pragma application_id = 0x1234");

    auto results = db->exec<int>("pragma application_id");
    EXPECT_EQ(results.singleValue(), 0x1234);
  }
}

}  // namespace database_tests
