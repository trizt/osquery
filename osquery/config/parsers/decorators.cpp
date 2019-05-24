/*
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include <osquery/config.h>
#include <osquery/flags.h>
#include <osquery/logger.h>
#include <osquery/sql.h>

#include "osquery/config/parsers/decorators.h"

namespace pt = boost::property_tree;

namespace osquery {

FLAG(bool, disable_decorators, false, "Disable log result decoration");

FLAG(bool,
     decorations_top_level,
     false,
     "Add decorators as top level JSON objects");

/// Statically define the parser name to avoid mistakes.
#define PARSER_NAME "decorators"

const std::map<DecorationPoint, std::string> kDecorationPointKeys = {
    {DECORATE_LOAD, "load"},
    {DECORATE_ALWAYS, "always"},
    {DECORATE_INTERVAL, "interval"},
};

using KeyValueMap = std::map<std::string, std::string>;
using DecorationStore = std::map<std::string, KeyValueMap>;

namespace {

/**
 * @brief A simple ConfigParserPlugin for a "decorators" dictionary key.
 *
 * Decorators append data to results, snapshots, and status log lines.
 * They can be used to add arbitrary additional datums within the 'decorators'
 * subkey.
 *
 * Decorators come in three basic flavors, defined by when they are run:
 * load: run these decorators when the config is loaded.
 * always: run these decorators for every query immediate before
 * interval: run these decorators on an interval.
 *
 * When 'interval' is used, the value is a dictionary of intervals, each of the
 * subkeys are treated as the requested interval in sections. The internals
 * are emulated by the query schedule.
 *
 * Decorators are sets of queries, and each selected column within the set is
 * added to the 'decorators' dictionary. Including two queries with the same
 * column name is undefined behavior and will most likely lead to either
 * duplicate keys or overwriting. Issuing a query that emits more than one row
 * will also lead to undefined behavior. The decorator executor will ignore any
 * rows past the first.
 */
class DecoratorsConfigParserPlugin : public ConfigParserPlugin {
 public:
  std::vector<std::string> keys() const override {
    return {PARSER_NAME};
  }

  Status setUp() override;

  Status update(const std::string& source, const ParserConfig& config) override;

  /// Update the set of decorators for a given source.
  void updateDecorations(const std::string& source,
                         const pt::ptree& decorators);

  /// Clear the decorations created from decorators for the given source.
  void clearSources(const std::string& source);

  /// Clear all decorations.
  virtual void reset() override;

 public:
  /// Set of configuration sources to the set of decorator queries.
  std::map<std::string, std::vector<std::string>> always_;

  /// Set of configuration sources to the set of on-load decorator queries.
  std::map<std::string, std::vector<std::string>> load_;

  /// Set of configuration sources to valid intervals.
  std::map<std::string, std::map<size_t, std::vector<std::string>>> intervals_;

 public:
  /// The result set of decorations, column names and their values.
  static DecorationStore kDecorations;

  /// Protect additions to the decorator set.
  static Mutex kDecorationsMutex;

  /// Protect the configuration controlled content.
  static Mutex kDecorationsConfigMutex;
};
}

DecorationStore DecoratorsConfigParserPlugin::kDecorations;
Mutex DecoratorsConfigParserPlugin::kDecorationsMutex;
Mutex DecoratorsConfigParserPlugin::kDecorationsConfigMutex;

Status DecoratorsConfigParserPlugin::setUp() {
  // Decorators are kept within customized data structures.
  // No need to define a key for the ::getData API.
  return Status(0, "OK");
}

Status DecoratorsConfigParserPlugin::update(const std::string& source,
                                            const ParserConfig& config) {
  clearSources(source);
  clearDecorations(source);
  if (config.count(PARSER_NAME) > 0) {
    // Each of these methods acquires the decorator lock separately.
    // The run decorators method is designed to have call sites throughout
    // the code base.
    updateDecorations(source, config.at(PARSER_NAME));
    runDecorators(DECORATE_LOAD, 0, source);
  }

  return Status(0, "OK");
}

void DecoratorsConfigParserPlugin::clearSources(const std::string& source) {
  // Reset the internal data store.
  WriteLock lock(DecoratorsConfigParserPlugin::kDecorationsConfigMutex);
  if (intervals_.count(source) > 0) {
    intervals_[source].clear();
  }

  if (always_.count(source) > 0) {
    always_[source].clear();
  }

  if (load_.count(source) > 0) {
    load_[source].clear();
  }
}

void DecoratorsConfigParserPlugin::reset() {
  // Reset the internal data store (for all sources).
  for (const auto& source : DecoratorsConfigParserPlugin::kDecorations) {
    clearSources(source.first);
    clearDecorations(source.first);
  }
}

void DecoratorsConfigParserPlugin::updateDecorations(
    const std::string& source, const pt::ptree& decorators) {
  WriteLock lock(DecoratorsConfigParserPlugin::kDecorationsConfigMutex);
  // Assign load decorators.
  auto& load_key = kDecorationPointKeys.at(DECORATE_LOAD);
  if (decorators.count(load_key) > 0) {
    for (const auto& item : decorators.get_child(load_key)) {
      load_[source].push_back(item.second.data());
    }
  }

  // Assign always decorators.
  auto& always_key = kDecorationPointKeys.at(DECORATE_ALWAYS);
  if (decorators.count(always_key) > 0) {
    for (const auto& item : decorators.get_child(always_key)) {
      always_[source].push_back(item.second.data());
    }
  }

  // Check if intervals are defined.
  auto& interval_key = kDecorationPointKeys.at(DECORATE_INTERVAL);
  if (decorators.count(interval_key) > 0) {
    auto& interval = decorators.get_child(interval_key);
    for (const auto& item : interval) {
      size_t rate = std::stoll(item.first);
      if (rate % 60 != 0) {
        LOG(WARNING) << "Invalid decorator interval rate " << rate
                     << " in config source: " << source;
        continue;
      }

      // This is a valid interval, update the set of intervals to include
      // this value. When intervals are checked this set is scanned, if a
      // match is found, then the associated config data is executed.
      for (const auto& interval_query : item.second) {
        intervals_[source][rate].push_back(interval_query.second.data());
      }
    }
  }
}

inline void addDecoration(const std::string& source,
                          const std::string& name,
                          const std::string& value) {
  WriteLock lock(DecoratorsConfigParserPlugin::kDecorationsMutex);
  DecoratorsConfigParserPlugin::kDecorations[source][name] = value;
}

inline void runDecorators(const std::string& source,
                          const std::vector<std::string>& queries) {
  for (const auto& query : queries) {
    SQL results(query);
    if (results.rows().size() > 0) {
      // Notice the warning above about undefined behavior when:
      // 1: You include decorators that emit the same column name
      // 2: You include a query that returns more than 1 row.
      for (const auto& column : results.rows()[0]) {
        addDecoration(source, column.first, column.second);
      }
    }

    if (results.rows().size() > 1) {
      // Multiple rows exhibit undefined behavior.
      LOG(WARNING) << "Multiple rows returned for decorator query: " << query;
    }
  }
}

void clearDecorations(const std::string& source) {
  WriteLock lock(DecoratorsConfigParserPlugin::kDecorationsMutex);
  DecoratorsConfigParserPlugin::kDecorations[source].clear();
}

void runDecorators(DecorationPoint point,
                   size_t time,
                   const std::string& source) {
  if (FLAGS_disable_decorators) {
    return;
  }

  auto parser = Config::getParser(PARSER_NAME);
  if (parser == nullptr) {
    // The decorators parser does not exist.
    return;
  }

  // Abstract the use of the decorator parser API.
  ReadLock lock(DecoratorsConfigParserPlugin::kDecorationsConfigMutex);
  auto dp = std::dynamic_pointer_cast<DecoratorsConfigParserPlugin>(parser);
  if (point == DECORATE_LOAD) {
    for (const auto& target_source : dp->load_) {
      if (source.empty() || target_source.first == source) {
        runDecorators(target_source.first, target_source.second);
      }
    }
  } else if (point == DECORATE_ALWAYS) {
    for (const auto& target_source : dp->always_) {
      if (source.empty() || target_source.first == source) {
        runDecorators(target_source.first, target_source.second);
      }
    }
  } else if (point == DECORATE_INTERVAL) {
    for (const auto& target_source : dp->intervals_) {
      for (const auto& interval : target_source.second) {
        if (time % interval.first == 0) {
          if (source.empty() || target_source.first == source) {
            runDecorators(target_source.first, interval.second);
          }
        }
      }
    }
  }
}

void getDecorations(std::map<std::string, std::string>& results) {
  if (FLAGS_disable_decorators) {
    return;
  }

  ReadLock lock(DecoratorsConfigParserPlugin::kDecorationsMutex);
  // Copy the decorations into the log_item.
  for (const auto& source : DecoratorsConfigParserPlugin::kDecorations) {
    for (const auto& decoration : source.second) {
      results[decoration.first] = decoration.second;
    }
  }
}

REGISTER_INTERNAL(DecoratorsConfigParserPlugin, "config_parser", PARSER_NAME);
}
