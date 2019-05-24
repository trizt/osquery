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
#include <osquery/core.h>
#include <osquery/events.h>
#include <osquery/extensions.h>
#include <osquery/filesystem.h>
#include <osquery/flags.h>
#include <osquery/logger.h>
#include <osquery/packs.h>
#include <osquery/registry.h>
#include <osquery/sql.h>
#include <osquery/system.h>
#include <osquery/tables.h>

#include "osquery/core/process.h"

namespace osquery {

DECLARE_bool(disable_logging);
DECLARE_bool(disable_events);

namespace tables {

QueryData genOsqueryEvents(QueryContext& context) {
  QueryData results;

  auto publishers = EventFactory::publisherTypes();
  for (const auto& publisher : publishers) {
    Row r;
    r["name"] = publisher;
    r["publisher"] = publisher;
    r["type"] = "publisher";

    auto pubref = EventFactory::getEventPublisher(publisher);
    if (pubref != nullptr) {
      r["subscriptions"] = INTEGER(pubref->numSubscriptions());
      r["events"] = INTEGER(pubref->numEvents());
      r["refreshes"] = INTEGER(pubref->restartCount());
      r["active"] = (pubref->hasStarted() && !pubref->isEnding()) ? "1" : "0";
    } else {
      r["subscriptions"] = "0";
      r["events"] = "0";
      r["refreshes"] = "0";
      r["active"] = "-1";
    }
    results.push_back(r);
  }

  auto subscribers = EventFactory::subscriberNames();
  for (const auto& subscriber : subscribers) {
    Row r;
    r["name"] = subscriber;
    r["type"] = "subscriber";
    // Subscribers will never 'restart'.
    r["refreshes"] = "0";

    auto subref = EventFactory::getEventSubscriber(subscriber);
    if (subref != nullptr) {
      r["publisher"] = subref->getType();
      r["subscriptions"] = INTEGER(subref->numSubscriptions());
      r["events"] = INTEGER(subref->numEvents());

      // Subscribers are always active, even if their publisher is not.
      r["active"] = (subref->state() == EventState::EVENT_RUNNING) ? "1" : "0";
    } else {
      r["subscriptions"] = "0";
      r["events"] = "0";
      r["active"] = "-1";
    }
    results.push_back(r);
  }

  return results;
}

QueryData genOsqueryPacks(QueryContext& context) {
  QueryData results;

  Config::getInstance().packs([&results](std::shared_ptr<Pack>& pack) {
    Row r;
    r["name"] = pack->getName();
    r["version"] = pack->getVersion();
    r["platform"] = pack->getPlatform();
    r["shard"] = INTEGER(pack->getShard());
    r["active"] = INTEGER(pack->isActive() ? 1 : 0);

    auto stats = pack->getStats();
    r["discovery_cache_hits"] = INTEGER(stats.hits);
    r["discovery_executions"] = INTEGER(stats.misses);
    results.push_back(r);
  });

  return results;
}

void genFlag(const std::string& name,
             const FlagInfo& flag,
             QueryData& results) {
  Row r;
  r["name"] = name;
  r["type"] = flag.type;
  r["description"] = flag.description;
  r["default_value"] = flag.default_value;
  r["value"] = flag.value;
  r["shell_only"] = (flag.detail.shell) ? "1" : "0";
  results.push_back(r);
}

QueryData genOsqueryFlags(QueryContext& context) {
  QueryData results;

  auto flags = Flag::flags();
  for (const auto& flag : flags) {
    if (flag.first.size() > 2) {
      // Skip single-character flags.
      genFlag(flag.first, flag.second, results);
    }
  }

  return results;
}

QueryData genOsqueryRegistry(QueryContext& context) {
  QueryData results;

  auto isActive = [](const std::string& plugin,
                     const std::shared_ptr<RegistryHelperCore>& registry) {
    if (FLAGS_disable_logging && registry->getName() == "logger") {
      return false;
    } else if (FLAGS_disable_events &&
               registry->getName().find("event") != std::string::npos) {
      return false;
    }

    const auto& active = registry->getActive();
    bool none_active = (active.empty());
    return (none_active || plugin == active);
  };

  const auto& registries = RegistryFactory::all();
  for (const auto& registry : registries) {
    const auto& plugins = registry.second->all();
    for (const auto& plugin : plugins) {
      Row r;
      r["registry"] = registry.first;
      r["name"] = plugin.first;
      r["owner_uuid"] = "0";
      r["internal"] = (registry.second->isInternal(plugin.first)) ? "1" : "0";
      r["active"] = (isActive(plugin.first, registry.second)) ? "1" : "0";
      results.push_back(r);
    }

    for (const auto& route : registry.second->getExternal()) {
      Row r;
      r["registry"] = registry.first;
      r["name"] = route.first;
      r["owner_uuid"] = INTEGER(route.second);
      r["internal"] = "0";
      r["active"] = (isActive(route.first, registry.second)) ? "1" : "0";
      results.push_back(r);
    }
  }

  return results;
}

QueryData genOsqueryExtensions(QueryContext& context) {
  QueryData results;

  ExtensionList extensions;
  if (getExtensions(extensions).ok()) {
    for (const auto& extension : extensions) {
      Row r;
      r["uuid"] = SQL_TEXT(extension.first);
      r["name"] = extension.second.name;
      r["version"] = extension.second.version;
      r["sdk_version"] = extension.second.sdk_version;
      r["path"] = getExtensionSocket(extension.first);
      r["type"] = (extension.first == 0) ? "core" : "extension";
      results.push_back(r);
    }
  }

  const auto& modules = RegistryFactory::getModules();
  for (const auto& module : modules) {
    Row r;
    r["uuid"] = SQL_TEXT(module.first);
    r["name"] = module.second.name;
    r["version"] = module.second.version;
    r["sdk_version"] = module.second.sdk_version;
    r["path"] = module.second.path;
    r["type"] = "module";
    results.push_back(r);
  }

  return results;
}

QueryData genOsqueryInfo(QueryContext& context) {
  QueryData results;

  Row r;
  r["pid"] = INTEGER(PlatformProcess::getCurrentProcess()->pid());
  r["version"] = kVersion;

  std::string hash_string;
  auto s = Config::getInstance().genHash(hash_string);
  r["config_hash"] = (s.ok()) ? hash_string : "";
  r["config_valid"] = Config::getInstance().isValid() ? INTEGER(1) : INTEGER(0);
  r["extensions"] =
      (pingExtension(FLAGS_extensions_socket).ok()) ? "active" : "inactive";
  r["build_platform"] = STR(OSQUERY_BUILD_PLATFORM);
  r["build_distro"] = STR(OSQUERY_BUILD_DISTRO);
  r["start_time"] = INTEGER(Config::getInstance().getStartTime());
  if (Initializer::isWorker()) {
    r["watcher"] = INTEGER(PlatformProcess::getLauncherProcess()->pid());
  } else {
    r["watcher"] = "-1";
  }

  results.push_back(r);
  return results;
}

QueryData genOsquerySchedule(QueryContext& context) {
  QueryData results;

  Config::getInstance().scheduledQueries(
      [&results](const std::string& name, const ScheduledQuery& query) {
        Row r;
        r["name"] = SQL_TEXT(name);
        r["query"] = SQL_TEXT(query.query);
        r["interval"] = INTEGER(query.interval);
        // Set default (0) values for each query if it has not yet executed.
        r["executions"] = "0";
        r["output_size"] = "0";
        r["wall_time"] = "0";
        r["user_time"] = "0";
        r["system_time"] = "0";
        r["average_memory"] = "0";
        r["last_executed"] = "0";

        // Report optional performance information.
        Config::getInstance().getPerformanceStats(
            name, [&r](const QueryPerformance& perf) {
              r["executions"] = BIGINT(perf.executions);
              r["last_executed"] = BIGINT(perf.last_executed);
              r["output_size"] = BIGINT(perf.output_size);
              r["wall_time"] = BIGINT(perf.wall_time);
              r["user_time"] = BIGINT(perf.user_time);
              r["system_time"] = BIGINT(perf.system_time);
              r["average_memory"] = BIGINT(perf.average_memory);
            });

        results.push_back(r);
      });
  return results;
}
}
}
