/*
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include <unistd.h>

#include <boost/algorithm/string.hpp>

#include <osquery/filesystem.h>
#include <osquery/sql.h>
#include <osquery/system.h>
#include <osquery/tables.h>

namespace osquery {
namespace tables {

QueryData genOSVersion(QueryContext& context) {
  Row r;

  r["major"] = "10";
  r["minor"] = "2";
  r["patch"] = "";
  r["name"] = "";
  r["build"] = "RELEASE";
  return {r};
}

QueryData genSystemInfo(QueryContext& context) {
  Row r;
  r["hostname"] = osquery::getHostname();
  r["computer_name"] = r["hostname"];

  std::string uuid;
  r["uuid"] = (osquery::getHostUUID(uuid)) ? uuid : "";

  auto qd = SQL::selectAllFrom("cpuid");
  for (const auto& row : qd) {
    if (row.at("feature") == "product_name") {
      r["cpu_brand"] = row.at("value");
      boost::trim(r["cpu_brand"]);
    }
  }

  // Can parse /proc/cpuinfo or /proc/meminfo for this data.
  static long cores = sysconf(_SC_NPROCESSORS_CONF);
  if (cores > 0) {
    r["cpu_logical_cores"] = INTEGER(cores);
    r["cpu_physical_cores"] = INTEGER(cores);
  } else {
    r["cpu_logical_cores"] = "-1";
    r["cpu_physical_cores"] = "-1";
  }

  static long pages = sysconf(_SC_PHYS_PAGES);
  static long pagesize = sysconf(_SC_PAGESIZE);

  if (pages > 0 && pagesize > 0) {
    r["physical_memory"] = BIGINT((long long)pages * (long long)pagesize);
  } else {
    r["physical_memory"] = "-1";
  }

  r["cpu_type"] = "0";
  r["cpu_subtype"] = "0";

  return {r};
}

QueryData genPlatformInfo(QueryContext& context) {
  return QueryData();
}
}
}
