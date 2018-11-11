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

#include "database.h"

#include <third_party/sqlite/sqlite3.h>

namespace db {

Statement::Statement(::sqlite3* db, const string& sql_statement) {
  int rc = sqlite3_prepare_v2(
      db, sql_statement.c_str(), int(sql_statement.size() + 1), &stmt_, nullptr);

  if (rc != SQLITE_OK || stmt_ == nullptr) {
    sqlite3_finalize(stmt_);
    throw core::Exception("Can't prepare SQL statement: '%s'", sql_statement);
  }
}

Statement::~Statement() {
  sqlite3_finalize(stmt_);
}

void Statement::bindValue(int index, std::nullopt_t) {
  if (sqlite3_bind_null(stmt_, index) != SQLITE_OK)
    throw core::Exception("Failed to bind SQL parameter");
}

void Statement::bindValue(int index, int value) {
  if (sqlite3_bind_int(stmt_, index, value) != SQLITE_OK)
    throw core::Exception("Failed to bind SQL parameter");
}

void Statement::bindValue(int index, int64_t value) {
  if (sqlite3_bind_int64(stmt_, index, value) != SQLITE_OK)
    throw core::Exception("Failed to bind SQL parameter");
}

void Statement::bindValue(int index, const string& value) {
  if (sqlite3_bind_text(
          stmt_, index, value.c_str(), int(value.size()), SQLITE_TRANSIENT) !=
      SQLITE_OK) {
    throw core::Exception("Failed to bind SQL parameter");
  }
}

void Statement::bindValue(int index, const char* value) {
  CHECK(value != nullptr, "use std::nullopt to set NULL values");

  if (sqlite3_bind_text(stmt_, index, value, -1, SQLITE_TRANSIENT) != SQLITE_OK) {
    throw core::Exception("Failed to bind SQL parameter");
  }
}

void Statement::bindValue(int index, double value) {
  if (sqlite3_bind_double(stmt_, index, value) != SQLITE_OK)
    throw core::Exception("Failed to bind SQL parameter");
}

bool Statement::step() {
  int rc = sqlite3_step(stmt_);
  switch (rc) {
    case SQLITE_DONE:
      return false;

    case SQLITE_ROW:
      return true;

    default:
      throw core::Exception(
          "Can't execute SQL statement (error %d: %s)", rc, sqlite3_errstr(rc));
  }
}

int Statement::columnCount() const {
  return sqlite3_column_count(stmt_);
}

void Statement::columnValue(int column, optional<int>& value) const {
  if (column >= columnCount())
    throw core::Exception("Invalid column index");

  switch (sqlite3_column_type(stmt_, column)) {
    case SQLITE_INTEGER:
      value = sqlite3_column_int(stmt_, column);
      break;

    case SQLITE_NULL:
      value.reset();
      break;

    default:
      throw core::Exception("Unexpected column data type");
  }
}

void Statement::columnValue(int column, optional<int64_t>& value) const {
  if (column >= columnCount())
    throw core::Exception("Invalid column index");

  switch (sqlite3_column_type(stmt_, column)) {
    case SQLITE_INTEGER:
      value = sqlite3_column_int64(stmt_, column);
      break;

    case SQLITE_NULL:
      value.reset();
      break;

    default:
      throw core::Exception("Unexpected column data type");
  }
}

void Statement::columnValue(int column, optional<string>& value) const {
  if (column >= columnCount())
    throw core::Exception("Invalid column index");

  switch (sqlite3_column_type(stmt_, column)) {
    case SQLITE_TEXT:
      value = reinterpret_cast<const char*>(sqlite3_column_text(stmt_, column));
      break;

    case SQLITE_NULL:
      value.reset();
      break;

    default:
      throw core::Exception("Unexpected column data type");
  }
}

void Statement::columnValue(int column, optional<double>& value) const {
  if (column >= columnCount())
    throw core::Exception("Invalid column index");

  switch (sqlite3_column_type(stmt_, column)) {
    case SQLITE_FLOAT:
      value = sqlite3_column_double(stmt_, column);
      break;

    case SQLITE_NULL:
      value.reset();
      break;

    default:
      throw core::Exception("Unexpected column data type");
  }
}

static int schemaVersionCheck(void* data, int argc, char* argv[], char*[]) {
  CHECK(data == nullptr);
  return (argc == 1 && string(argv[0]) == "0") ? 0 : SQLITE_ERROR;
}

Connection::Connection(const string& filename, OpenMode open_mode, int busy_wait_ms) {
  CHECK(busy_wait_ms >= 0);

  int flags = 0;

  switch (open_mode) {
    case OpenMode::CreateNew:
      flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
      break;

    case OpenMode::ExistingDatabase:
      flags = SQLITE_OPEN_READWRITE;
      break;

    default:
      FATAL("Unexpected OpenMode");
  }

  if (sqlite3_open_v2(filename.c_str(), &db_, flags, nullptr) != SQLITE_OK) {
    CHECK(sqlite3_close(db_) == SQLITE_OK);
    throw core::Exception("Failed to open database: '%s'", filename.c_str());
  }

  if (busy_wait_ms > 0) {
    CHECK(sqlite3_busy_timeout(db_, busy_wait_ms) == SQLITE_OK);
  }

  // sqlite3 does not provide an "create new" database (ie. fail if the
  // database already exists) so we need to do our best to aproximate it
  // (checking if schema_version == 0 is close enough but not a perfect solution)
  if (open_mode == OpenMode::CreateNew) {
    if (sqlite3_exec(
            db_, "PRAGMA schema_version", &schemaVersionCheck, nullptr, nullptr) !=
        SQLITE_OK) {
      CHECK(sqlite3_close(db_) == SQLITE_OK);
      throw core::Exception("Not a brand new database: '%s'", filename.c_str());
    }
  }
}

Connection::~Connection() {
  CHECK(sqlite3_close(db_) == SQLITE_OK);
}

void Connection::beginTransaction(TransactionOption option) {
  switch (option) {
    case TransactionOption::Deferred:
      exec("BEGIN DEFERRED TRANSACTION");
      break;

    case TransactionOption::Immediate:
      exec("BEGIN IMMEDIATE TRANSACTION");
      break;

    case TransactionOption::Exclusive:
      exec("BEGIN EXCLUSIVE TRANSACTION");
      break;

    default:
      FATAL("Unexpected transaction option");
  }
}

void Connection::commit() {
  exec("COMMIT TRANSACTION");
}

void Connection::rollback() {
  exec("ROLLBACK TRANSACTION");
}

RowId Connection::lastInsertRowId() const {
  return sqlite3_last_insert_rowid(db_);
}

TransactionScope::TransactionScope(Connection& database, TransactionOption option)
    : database_(database) {
  database_.beginTransaction(option);
}

TransactionScope::~TransactionScope() {
  if (!completed_)
    database_.rollback();
}

void TransactionScope::commit() {
  CHECK(!completed_);
  database_.commit();
  completed_ = true;
}

void TransactionScope::rollback() {
  CHECK(!completed_);
  database_.rollback();
  completed_ = true;
}

}  // namespace db
