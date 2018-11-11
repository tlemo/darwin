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

#include <cstddef>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
using namespace std;

// CONSIDER:
// - lazy rezultset / cursor (call step on demand)
// - aggregate type mapping (for params/results, ex. exec<MyStruct>(...))

struct sqlite3;
struct sqlite3_stmt;

namespace db {

//! Open database flag
enum class OpenMode {
  ExistingDatabase,     //!< The database file must exist
  CreateNew             //!< Creates a new database (will fail if file already exists)
};

//! Represents the ID of a row in the database
using RowId = int64_t;

//! A prepared Sqlite statement
class Statement {
 public:
  explicit Statement(::sqlite3* db, const string& sql_statement);
  ~Statement();

  // no copy/move semantics
  Statement(const Statement&) = delete;
  Statement& operator=(const Statement&) = delete;

  template <class... PARAMS>
  void bind(PARAMS&&... params) {
    bindHelper(1, std::forward<PARAMS>(params)...);
  }

  // statement parameter binding
  // (normally done through the bind helper)
  void bindValue(int index, std::nullopt_t);
  void bindValue(int index, int value);
  void bindValue(int index, int64_t value);
  void bindValue(int index, const string& value);
  void bindValue(int index, const char* value);
  void bindValue(int index, double value);

  // use std::nullopt for NULL values
  void bindValue(int index, nullptr_t) = delete;

  template <class T>
  void bindValue(int index, const optional<T>& value) {
    if (value.has_value())
      bindValue(index, *value);
    else
      bindValue(index, nullopt);
  }

  // returns true if result row available
  // (and false if it's done)
  bool step();

  int columnCount() const;

  // current result row accessors
  // (normally used by a ResultSet)
  void columnValue(int column, optional<int>& value) const;
  void columnValue(int column, optional<int64_t>& value) const;
  void columnValue(int column, optional<string>& value) const;
  void columnValue(int column, optional<double>& value) const;

 private:
  template <class T, class... PARAMS>
  void bindHelper(int index, T&& value, PARAMS&&... params) {
    bindValue(index, std::forward<T>(value));
    bindHelper(index + 1, std::forward<PARAMS>(params)...);
  }

  void bindHelper(int /*index*/) {}

 private:
  ::sqlite3_stmt* stmt_ = nullptr;
};

//! Represents the results of executing a query
//! \sa db::Connection::exec()
template <class... TYPES>
class ResultSet {
 public:
  //! A results row
  using Row = tuple<optional<TYPES>...>;

 public:
  //! Indexed access to the rows in the result
  const Row& operator[](size_t index) const { return results_[index]; }

  //! Count of columns in the results
  int columnCount() const { return tuple_size<Row>::value; }

  //! Count of rows in the results
  size_t size() const { return results_.size(); }
  
  //! Returns true if the ResultSet is empty
  bool empty() const { return results_.empty(); }
  
  //! ResultSet begin iterator
  auto begin() const { return results_.begin(); }

  //! ResultSet end iterator
  auto end() const { return results_.end(); }

  //! Convenience helper for extracting a single value
  //! \throws core::Exception if the ResultSet has more than one value (one row/col)
  const auto& singleValue() const {
    if (tuple_size<Row>::value != 1 || results_.size() != 1)
      throw core::Exception("ResultSet is not a single value");
    return get<0>(results_[0]);
  }

  void extractRow(const Statement& statement) {
    if (columnCount() > 0 && statement.columnCount() != columnCount()) {
      throw core::Exception(
          "ResultSet column count does not match the statement column count");
    }

    Row row;
    extractColumns(statement, row, index_sequence_for<TYPES...>{});
    results_.push_back(std::move(row));
  }

 private:
  template <size_t... COLUMNS>
  void extractColumns([[maybe_unused]] const Statement& statement,
                      [[maybe_unused]] Row& row,
                      index_sequence<COLUMNS...>) {
    (statement.columnValue(COLUMNS, std::get<COLUMNS>(row)), ...);
  }

 private:
  vector<Row> results_;
};

//! The type of Sqlite transaction
//! 
//! \sa TransactionScope
//! 
//! \todo Consider a per-connection isolation_mode (instead of TransactionOption)
//! 
enum class TransactionOption {
  Deferred,   //!< Maps to _BEGIN DEFERRED TRANSACTION_
  Immediate,  //!< Maps to _BEGIN IMMEDIATE TRANSACTION_
  Exclusive   //!< Maps to _BEGIN EXCLUSIVE TRANSACTION_
};

//! A very simple relational database abstraction on top of Sqlite
class Connection : public core::NonCopyable {
 public:
  //! Opens a Sqlite connection
  explicit Connection(const string& filename, OpenMode open_mode, int busy_wait_ms = 500);

  ~Connection();

  //! Starts a transaction
  void beginTransaction(TransactionOption option = TransactionOption::Deferred);
  
  //! Commits the current transaction
  void commit();
  
  //! Aborts the current transaction
  void rollback();

  //! Returns the ID of the last inserted row with this connection
  RowId lastInsertRowId() const;

  // TODO: consider stm = prepare() / exec(stm)

  //! Executes the specified Sqlite statement and returns the results as a ResultSet
  //! 
  //! It supports _parameterized_ queries: each instance of ? is substituted by the
  //! corresponding parameter value:
  //! 
  //! ```cpp
  //! exec("insert into t(name, value) values(?, ?)", "Darwin", 100);
  //! ```
  //! 
  template <class... RESULTS, class... PARAMS>
  ResultSet<RESULTS...> exec(const string& sql_statement, PARAMS&&... params) {
    Statement prepared_statement(db_, sql_statement);
    prepared_statement.bind(std::forward<PARAMS>(params)...);

    ResultSet<RESULTS...> results;
    while (prepared_statement.step())
      results.extractRow(prepared_statement);
    return results;
  }

 private:
  ::sqlite3* db_ = nullptr;
};

//! A scope-based transaction guard
class TransactionScope {
 public:
  //! Creates a transaction guard
  explicit TransactionScope(Connection& databaseTransaction,
                            TransactionOption option = TransactionOption::Deferred);

  //! Unless the transaction was explicitly committed, it will rollback
  //! \sa commit()
  ~TransactionScope();

  //! Explicitly commit the transaction
  void commit();
  
  //! Explicitly rolls back the transaction
  void rollback();

 private:
  Connection& database_;
  bool completed_ = false;
};

}  // namespace db
