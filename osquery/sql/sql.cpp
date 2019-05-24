/*
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include <sstream>

#include <osquery/core.h>
#include <osquery/logger.h>
#include <osquery/registry.h>
#include <osquery/sql.h>
#include <osquery/tables.h>

namespace osquery {

FLAG(int32, value_max, 512, "Maximum returned row value size");

CREATE_LAZY_REGISTRY(SQLPlugin, "sql");

SQL::SQL(const std::string& q) {
  TableColumns table_columns;
  q_ = q;
  status_ = getQueryColumns(q_, table_columns);
  if (status_.ok()) {
    for (auto c : table_columns) {
      columns_.push_back(std::get<0>(c));
    }
    status_ = query(q_, results_);
  }
}

const QueryData& SQL::rows() const {
  return results_;
}

const ColumnNames& SQL::columns() {
  return columns_;
}

bool SQL::ok() {
  return status_.ok();
}

const Status& SQL::getStatus() const {
  return status_;
}

std::string SQL::getMessageString() {
  return status_.toString();
}

static inline void escapeNonPrintableBytes(std::string& data) {
  std::string escaped;
  // clang-format off
  char const hex_chars[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
  };
  // clang-format on

  bool needs_replacement = false;
  for (size_t i = 0; i < data.length(); i++) {
    if (((unsigned char)data[i]) < 0x20 || ((unsigned char)data[i]) >= 0x80) {
      needs_replacement = true;
      escaped += "\\x";
      escaped += hex_chars[(((unsigned char)data[i])) >> 4];
      escaped += hex_chars[((unsigned char)data[i] & 0x0F) >> 0];
    } else {
      escaped += data[i];
    }
  }

  // Only replace if any escapes were made.
  if (needs_replacement) {
    data = std::move(escaped);
  }
}

void escapeNonPrintableBytesEx(std::string& data) {
  return escapeNonPrintableBytes(data);
}

void SQL::escapeResults() {
  for (auto& row : results_) {
    for (auto& column : row) {
      escapeNonPrintableBytes(column.second);
    }
  }
}

QueryData SQL::selectAllFrom(const std::string& table) {
  PluginResponse response;
  Registry::call("table", table, {{"action", "generate"}}, response);
  return response;
}

QueryData SQL::selectAllFrom(const std::string& table,
                             const std::string& column,
                             ConstraintOperator op,
                             const std::string& expr) {
  PluginRequest request = {{"action", "generate"}};
  {
    // Create a fake content, there will be no caching.
    QueryContext ctx;
    ctx.constraints[column].add(Constraint(op, expr));
    TablePlugin::setRequestFromContext(ctx, request);
  }

  PluginResponse response;
  Registry::call("table", table, request, response);
  return response;
}

Status SQLPlugin::call(const PluginRequest& request, PluginResponse& response) {
  response.clear();
  if (request.count("action") == 0) {
    return Status(1, "SQL plugin must include a request action");
  }

  if (request.at("action") == "query") {
    return this->query(request.at("query"), response);
  } else if (request.at("action") == "columns") {
    TableColumns columns;
    auto status = this->getQueryColumns(request.at("query"), columns);
    // Convert columns to response
    for (const auto& column : columns) {
      response.push_back(
          {{"n", std::get<0>(column)},
           {"t", columnTypeName(std::get<1>(column))},
           {"o", INTEGER(static_cast<size_t>(std::get<2>(column)))}});
    }
    return status;
  } else if (request.at("action") == "attach") {
    // Attach a virtual table name using an optional included definition.
    return this->attach(request.at("table"));
  } else if (request.at("action") == "detach") {
    this->detach(request.at("table"));
    return Status(0, "OK");
  }
  return Status(1, "Unknown action");
}

Status query(const std::string& q, QueryData& results) {
  return Registry::call(
      "sql", "sql", {{"action", "query"}, {"query", q}}, results);
}

Status getQueryColumns(const std::string& q, TableColumns& columns) {
  PluginResponse response;
  auto status = Registry::call(
      "sql", "sql", {{"action", "columns"}, {"query", q}}, response);

  // Convert response to columns
  for (const auto& item : response) {
    columns.push_back(make_tuple(
        item.at("n"), columnTypeName(item.at("t")), ColumnOptions::DEFAULT));
  }
  return status;
}
}
