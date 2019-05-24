/**
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under both the Apache 2.0 license (found in the
 *  LICENSE file in the root directory of this source tree) and the GPLv2 (found
 *  in the COPYING file in the root directory of this source tree).
 *  You may select, at your option, one of the above-listed licenses.
 */

#include <boost/algorithm/string/predicate.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <regex>

#include <osquery/database.h>
#include <osquery/flags.h>
#include <osquery/logger.h>
#include <osquery/registry.h>

#include "osquery/core/conversions.h"
#include "osquery/core/flagalias.h"
#include "osquery/core/json.h"

namespace pt = boost::property_tree;
namespace rj = rapidjson;

namespace osquery {

/// Generate a specific-use registry for database access abstraction.
CREATE_REGISTRY(DatabasePlugin, "database");

CLI_FLAG(bool, database_dump, false, "Dump the contents of the backing store");

CLI_FLAG(string,
         database_path,
         OSQUERY_DB_HOME "/osquery.db",
         "If using a disk-based backing store, specify a path");
FLAG_ALIAS(std::string, db_path, database_path);

FLAG(bool, disable_database, false, "Disable the persistent RocksDB storage");

const std::string kInternalDatabase = "rocksdb";
const std::string kPersistentSettings = "configurations";
const std::string kQueries = "queries";
const std::string kEvents = "events";
const std::string kCarves = "carves";
const std::string kLogs = "logs";

const std::string kDbEpochSuffix = "epoch";
const std::string kDbCounterSuffix = "counter";

const std::string kDbVersionKey = "results_version";

const std::vector<std::string> kDomains = {
    kPersistentSettings, kQueries, kEvents, kLogs, kCarves};

std::atomic<bool> DatabasePlugin::kDBAllowOpen(false);
std::atomic<bool> DatabasePlugin::kDBRequireWrite(false);
std::atomic<bool> DatabasePlugin::kDBInitialized(false);
std::atomic<bool> DatabasePlugin::kDBChecking(false);

/**
 * @brief A reader/writer mutex protecting database resets.
 *
 * A write is locked while using reset flows. A read is locked when calling
 * database plugin APIs.
 */
Mutex kDatabaseReset;

Status DatabasePlugin::initPlugin() {
  // Initialize the database plugin using the flag.
  auto plugin = (FLAGS_disable_database) ? "ephemeral" : kInternalDatabase;
  auto status = RegistryFactory::get().setActive("database", plugin);
  if (!status.ok()) {
    // If the database did not setUp override the active plugin.
    RegistryFactory::get().setActive("database", "ephemeral");
  }

  kDBInitialized = true;
  return status;
}

void DatabasePlugin::shutdown() {
  auto datbase_registry = RegistryFactory::get().registry("database");
  for (auto& plugin : RegistryFactory::get().names("database")) {
    datbase_registry->remove(plugin);
  }
}

Status DatabasePlugin::reset() {
  // Keep this simple, scope the critical section to the broader methods.
  tearDown();
  return setUp();
}

bool DatabasePlugin::checkDB() {
  kDBChecking = true;
  bool result = true;
  try {
    auto status = setUp();
    if (kDBRequireWrite && read_only_) {
      result = false;
    }
    tearDown();
    result = status.ok();
  } catch (const std::exception& e) {
    VLOG(1) << "Database plugin check failed: " << e.what();
    result = false;
  }
  kDBChecking = false;
  return result;
}

Status DatabasePlugin::scan(const std::string& domain,
                            std::vector<std::string>& results,
                            const std::string& prefix,
                            size_t max) const {
  return Status(0, "Not used");
}

Status DatabasePlugin::call(const PluginRequest& request,
                            PluginResponse& response) {
  if (request.count("action") == 0) {
    return Status(1, "Database plugin must include a request action");
  }

  // Get a domain/key, which are used for most database plugin actions.
  auto domain = (request.count("domain") > 0) ? request.at("domain") : "";
  auto key = (request.count("key") > 0) ? request.at("key") : "";

  if (request.at("action") == "reset") {
    WriteLock lock(kDatabaseReset);
    DatabasePlugin::kDBInitialized = false;
    // Prevent RocksDB reentrancy by logger plugins during plugin setup.
    VLOG(1) << "Resetting the database plugin: " << getName();
    auto status = this->reset();
    if (!status.ok()) {
      // The active database could not be reset, fallback to an ephemeral.
      Registry::get().setActive("database", "ephemeral");
      LOG(WARNING) << "Unable to reset database plugin: " << getName();
    }
    DatabasePlugin::kDBInitialized = true;
    return status;
  }

  // Switch over the possible database plugin actions.
  ReadLock lock(kDatabaseReset);
  if (request.at("action") == "get") {
    std::string value;
    auto status = this->get(domain, key, value);
    response.push_back({{"v", value}});
    return status;
  } else if (request.at("action") == "put") {
    if (request.count("value") == 0) {
      return Status(1, "Database plugin put action requires a value");
    }
    return this->put(domain, key, request.at("value"));
  } else if (request.at("action") == "putBatch") {
    if (request.count("json") == 0) {
      return Status(
          1,
          "Database plugin putBatch action requires a json-encoded value list");
    }

    auto json_object = JSON::newObject();

    auto status = json_object.fromString(request.at("json"));
    if (!status.ok()) {
      VLOG(1) << status.getMessage();
      return status;
    }

    const auto& json_object_list = json_object.doc().GetObject();

    DatabaseStringValueList data;
    data.reserve(json_object_list.MemberCount());

    for (auto& item : json_object_list) {
      if (!item.value.IsString()) {
        return Status(1,
                      "Database plugin putBatch action with an invalid json "
                      "received. Only string values are supported");
      }

      data.push_back(
          std::make_pair(item.name.GetString(), item.value.GetString()));
    }

    return this->putBatch(domain, data);
  } else if (request.at("action") == "remove") {
    return this->remove(domain, key);
  } else if (request.at("action") == "remove_range") {
    auto key_high = (request.count("high") > 0) ? request.at("key_high") : "";
    if (!key_high.empty() && !key.empty()) {
      return this->removeRange(domain, key, key_high);
    }
    return Status(1, "Missing range");
  } else if (request.at("action") == "scan") {
    // Accumulate scanned keys into a vector.
    std::vector<std::string> keys;
    // Optionally allow the caller to request a max number of keys.
    size_t max = 0;
    if (request.count("max") > 0) {
      max = std::stoul(request.at("max"));
    }
    auto status = this->scan(domain, keys, request.at("prefix"), max);
    for (const auto& k : keys) {
      response.push_back({{"k", k}});
    }
    return status;
  }

  return Status(1, "Unknown database plugin action");
}

static inline std::shared_ptr<DatabasePlugin> getDatabasePlugin() {
  auto& rf = RegistryFactory::get();
  if (!rf.exists("database", rf.getActive("database"), true)) {
    return nullptr;
  }

  auto plugin = rf.plugin("database", rf.getActive("database"));
  return std::dynamic_pointer_cast<DatabasePlugin>(plugin);
}

namespace {
Status sendPutBatchDatabaseRequest(const std::string& domain,
                                   const DatabaseStringValueList& data) {
  auto json_object = JSON::newObject();
  for (const auto& p : data) {
    const auto& key = p.first;
    const auto& value = p.second;

    json_object.addRef(key, value);
  }

  std::string serialized_data;
  auto status = json_object.toString(serialized_data);
  if (!status.ok()) {
    VLOG(1) << status.getMessage();
    return status;
  }

  PluginRequest request = {{"action", "putBatch"},
                           {"domain", domain},
                           {"json", std::move(serialized_data)}};

  status = Registry::call("database", request);
  if (!status.ok()) {
    VLOG(1) << status.getMessage();
  }

  return status;
}

Status sendPutDatabaseRequest(const std::string& domain,
                              const DatabaseStringValueList& data) {
  const auto& key = data[0].first;
  const auto& value = data[1].second;

  PluginRequest request = {
      {"action", "put"}, {"domain", domain}, {"key", key}, {"value", value}};

  auto status = Registry::call("database", request);
  if (!status.ok()) {
    VLOG(1) << status.getMessage();
  }

  return status;
}
} // namespace

Status getDatabaseValue(const std::string& domain,
                        const std::string& key,
                        std::string& value) {
  if (domain.empty()) {
    return Status(1, "Missing domain");
  }

  if (RegistryFactory::get().external()) {
    // External registries (extensions) do not have databases active.
    // It is not possible to use an extension-based database.
    PluginRequest request = {
        {"action", "get"}, {"domain", domain}, {"key", key}};
    PluginResponse response;
    auto status = Registry::call("database", request, response);
    if (status.ok()) {
      // Set value from the internally-known "v" key.
      if (response.size() > 0 && response[0].count("v") > 0) {
        value = response[0].at("v");
      }
    }
    return status;
  }

  ReadLock lock(kDatabaseReset);
  if (!DatabasePlugin::kDBInitialized) {
    throw std::runtime_error("Cannot get database value: " + key);
  } else {
    auto plugin = getDatabasePlugin();
    return plugin->get(domain, key, value);
  }
}

Status getDatabaseValue(const std::string& domain,
                        const std::string& key,
                        int& value) {
  std::string result;
  auto s = getDatabaseValue(domain, key, result);
  if (s.ok()) {
    value = std::stoi(result);
  }
  return s;
}

Status setDatabaseValue(const std::string& domain,
                        const std::string& key,
                        const std::string& value) {
  return setDatabaseBatch(domain, {std::make_pair(key, value)});
}

Status setDatabaseBatch(const std::string& domain,
                        const DatabaseStringValueList& data) {
  if (domain.empty()) {
    return Status(1, "Missing domain");
  }

  // External registries (extensions) do not have databases active.
  // It is not possible to use an extension-based database.
  if (RegistryFactory::get().external()) {
    if (data.size() >= 1) {
      return sendPutBatchDatabaseRequest(domain, data);
    } else {
      return sendPutDatabaseRequest(domain, data);
    }
  }

  ReadLock lock(kDatabaseReset);
  if (!DatabasePlugin::kDBInitialized) {
    throw std::runtime_error("Cannot set database values");
  }

  auto plugin = getDatabasePlugin();
  return plugin->putBatch(domain, data);
}

Status setDatabaseValue(const std::string& domain,
                        const std::string& key,
                        int value) {
  return setDatabaseBatch(domain, {std::make_pair(key, std::to_string(value))});
}

Status deleteDatabaseValue(const std::string& domain, const std::string& key) {
  if (domain.empty()) {
    return Status(1, "Missing domain");
  }

  if (RegistryFactory::get().external()) {
    // External registries (extensions) do not have databases active.
    // It is not possible to use an extension-based database.
    PluginRequest request = {
        {"action", "remove"}, {"domain", domain}, {"key", key}};
    return Registry::call("database", request);
  }

  ReadLock lock(kDatabaseReset);
  if (!DatabasePlugin::kDBInitialized) {
    throw std::runtime_error("Cannot delete database value: " + key);
  } else {
    auto plugin = getDatabasePlugin();
    return plugin->remove(domain, key);
  }
}

Status deleteDatabaseRange(const std::string& domain,
                           const std::string& low,
                           const std::string& high) {
  if (domain.empty()) {
    return Status(1, "Missing domain");
  }

  if (RegistryFactory::get().external()) {
    // External registries (extensions) do not have databases active.
    // It is not possible to use an extension-based database.
    PluginRequest request = {{"action", "remove_range"},
                             {"domain", domain},
                             {"key", low},
                             {"key_high", high}};
    return Registry::call("database", request);
  }

  ReadLock lock(kDatabaseReset);
  if (!DatabasePlugin::kDBInitialized) {
    throw std::runtime_error("Cannot delete database values: " + low + " - " +
                             high);
  } else {
    auto plugin = getDatabasePlugin();
    return plugin->removeRange(domain, low, high);
  }
}

Status scanDatabaseKeys(const std::string& domain,
                        std::vector<std::string>& keys,
                        size_t max) {
  return scanDatabaseKeys(domain, keys, "", max);
}

/// Get a list of keys for a given domain.
Status scanDatabaseKeys(const std::string& domain,
                        std::vector<std::string>& keys,
                        const std::string& prefix,
                        size_t max) {
  if (domain.empty()) {
    return Status(1, "Missing domain");
  }

  if (RegistryFactory::get().external()) {
    // External registries (extensions) do not have databases active.
    // It is not possible to use an extension-based database.
    PluginRequest request = {{"action", "scan"},
                             {"domain", domain},
                             {"prefix", prefix},
                             {"max", std::to_string(max)}};
    PluginResponse response;
    auto status = Registry::call("database", request, response);

    for (const auto& item : response) {
      if (item.count("k") > 0) {
        keys.push_back(item.at("k"));
      }
    }
    return status;
  }

  ReadLock lock(kDatabaseReset);
  if (!DatabasePlugin::kDBInitialized) {
    throw std::runtime_error("Cannot scan database values: " + prefix);
  } else {
    auto plugin = getDatabasePlugin();
    return plugin->scan(domain, keys, prefix, max);
  }
}

void resetDatabase() {
  PluginRequest request = {{"action", "reset"}};
  Registry::call("database", request);
}

void dumpDatabase() {
  auto plugin = getDatabasePlugin();
  plugin->dumpDatabase();
}

Status ptreeToRapidJSON(const std::string& in, std::string& out) {
  pt::ptree tree;
  try {
    std::stringstream ss;
    ss << in;
    pt::read_json(ss, tree);
  } catch (const pt::json_parser::json_parser_error& /* e */) {
    return Status(1, "Failed to parse JSON");
  }

  auto json = JSON::newArray();
  for (const auto& t : tree) {
    std::stringstream ss;
    pt::write_json(ss, t.second);

    rj::Document row;
    if (row.Parse(ss.str()).HasParseError()) {
      return Status(1, "Failed to serialize JSON");
    }
    json.push(row);
  }

  json.toString(out);

  return Status();
}

static Status migrateV0V1(void) {
  std::vector<std::string> keys;
  auto s = scanDatabaseKeys(kQueries, keys);
  if (!s.ok()) {
    return Status(1, "Failed to lookup legacy query data from database");
  }

  for (const auto& key : keys) {
    // Skip over epoch and counter entries, as 0 is parsed by ptree
    if (boost::algorithm::ends_with(key, kDbEpochSuffix) ||
        boost::algorithm::ends_with(key, kDbCounterSuffix) ||
        boost::algorithm::starts_with(key, "query.")) {
      continue;
    }

    std::string value{""};
    if (!getDatabaseValue(kQueries, key, value)) {
      LOG(WARNING) << "Failed to get value from database " << key;
      continue;
    }

    std::string out;
    s = ptreeToRapidJSON(value, out);
    if (!s.ok()) {
      LOG(WARNING) << "Conversion from ptree to RapidJSON failed for '" << key
                   << ": " << value << "': " << s.what() << ". Dropping key!";
      continue;
    }

    if (!setDatabaseValue(kQueries, key, out)) {
      LOG(WARNING) << "Failed to update value in database " << key << ": "
                   << value;
    }
  }

  return Status();
}

static Status migrateV1V2(void) {
  std::vector<std::string> keys;
  std::regex re = std::regex("(.*)\\.audit\\.(.*)");

  Status s = scanDatabaseKeys(kEvents, keys);
  if (!s.ok()) {
    return Status::failure(
        1, "Failed to scan event keys from database: " + s.what());
  }

  for (const auto& key : keys) {
    std::smatch match;
    if (std::regex_match(key, match, re)) {
      std::string value;
      const std::string new_key =
          match[1].str() + ".auditeventpublisher." + match[2].str();

      s = getDatabaseValue(kEvents, key, value);
      if (!s.ok()) {
        LOG(ERROR) << "Failed to read value for key '" << key
                   << "'. Key will be kept but won't be migrated!";
        continue;
      }

      s = setDatabaseValue(kEvents, new_key, value);
      if (!s.ok()) {
        LOG(ERROR) << "Failed to set value for key '" << new_key
                   << "' migrated from '" << key
                   << "'. Original key will be kept but won't be migrated!";
        continue;
      }

      s = deleteDatabaseValue(kEvents, key);
      if (!s.ok()) {
        LOG(WARNING) << "Failed to delete key '" << key
                     << "' after migration to new key '" << new_key
                     << "'. Original key will be kept but data was migrated!";
      }
    }
  }

  return Status::success();
}

Status upgradeDatabase(int to_version) {
  LOG(INFO) << "Checking database version for migration";

  std::string value;
  Status st = getDatabaseValue(kPersistentSettings, kDbVersionKey, value);

  int db_version = 0;
  /* Since there isn't a reliable way to determined what happen when the read
   * fails we just assume the key doesn't exist which indicates database
   * version 0.
   */
  if (st.ok()) {
    auto ret = tryTo<int>(value);
    if (ret.isError()) {
      LOG(ERROR) << "Invalid value '" << value << "'for " << kDbVersionKey
                 << " key. Database is corrupted.";
      return Status(1, "Invalid value for database version.");
    } else {
      db_version = ret.get();
    }
  }

  while (db_version != to_version) {
    Status migrate_status;

    LOG(INFO) << "Performing migration: " << db_version << " -> "
              << (db_version + 1);

    switch (db_version) {
    case 0:
      migrate_status = migrateV0V1();
      break;

    case 1:
      migrate_status = migrateV1V2();
      break;

    default:
      LOG(ERROR) << "Logic error: the migration code is broken!";
      migrate_status = Status(1);
      break;
    }

    if (!migrate_status.ok()) {
      return Status(1, "Database migration failed.");
    }

    st = setDatabaseValue(
        kPersistentSettings, kDbVersionKey, std::to_string(db_version + 1));
    if (!st.ok()) {
      LOG(ERROR) << "Failed to set new database version after migration. "
                 << "The DB was correctly migrated from version " << db_version
                 << " to version " << (db_version + 1)
                 << " but persisting the new version failed.";
      return Status(1, "Database migration failed.");
    }

    LOG(INFO) << "Migration " << db_version << " -> " << (db_version + 1)
              << " successfully completed!";

    db_version++;
  }

  return Status();
}
} // namespace osquery
