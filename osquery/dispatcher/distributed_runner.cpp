/**
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under both the Apache 2.0 license (found in the
 *  LICENSE file in the root directory of this source tree) and the GPLv2 (found
 *  in the COPYING file in the root directory of this source tree).
 *  You may select, at your option, one of the above-listed licenses.
 */

#include <chrono>

#include <osquery/database.h>
#include <osquery/distributed.h>
#include <osquery/flags.h>
#include <osquery/system.h>

#include "osquery/core/conversions.h"
#include "osquery/dispatcher/distributed_runner.h"

namespace osquery {

FLAG(uint64,
     distributed_interval,
     60,
     "Seconds between polling for new queries (default 60)")

DECLARE_bool(disable_distributed);
DECLARE_string(distributed_plugin);

const size_t kDistributedAccelerationInterval = 5;

void DistributedRunner::start() {
  auto dist = Distributed();
  while (!interrupted()) {
    dist.pullUpdates();
    if (dist.getPendingQueryCount() > 0) {
      dist.runQueries();
    }

    std::string str_acu = "0";
    Status database = getDatabaseValue(
        kPersistentSettings, "distributed_accelerate_checkins_expire", str_acu);
    auto const accelerate_checkins_expire_exp =
        tryTo<unsigned long int>(str_acu, 10);
    if (!database.ok() || accelerate_checkins_expire_exp.isError() ||
        getUnixTime() > accelerate_checkins_expire_exp.get()) {
      pause(std::chrono::seconds(FLAGS_distributed_interval));
    } else {
      pause(std::chrono::seconds(kDistributedAccelerationInterval));
    }
  }
}

Status startDistributed() {
  if (!FLAGS_disable_distributed) {
    Dispatcher::addService(std::make_shared<DistributedRunner>());
    return Status(0, "OK");
  } else {
    return Status(1, "Distributed query service not enabled.");
  }
}
} // namespace osquery
