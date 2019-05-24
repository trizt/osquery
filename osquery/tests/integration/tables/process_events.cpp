
/**
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under both the Apache 2.0 license (found in the
 *  LICENSE file in the root directory of this source tree) and the GPLv2 (found
 *  in the COPYING file in the root directory of this source tree).
 *  You may select, at your option, one of the above-listed licenses.
 */

// Sanity check integration test for process_events
// Spec file: specs/posix/process_events.table

#include <osquery/tests/integration/tables/helper.h>

namespace osquery {

class processEvents : public IntegrationTableTest {};

TEST_F(processEvents, test_sanity) {
  // 1. Query data
  // QueryData data = execute_query("select * from process_events");
  // 2. Check size before validation
  // ASSERT_GE(data.size(), 0ul);
  // ASSERT_EQ(data.size(), 1ul);
  // ASSERT_EQ(data.size(), 0ul);
  // 3. Build validation map
  // See IntegrationTableTest.cpp for avaialbe flags
  // Or use custom DataCheck object
  // ValidatatioMap row_map = {
  //      {"pid", IntType}
  //      {"path", NormalType}
  //      {"mode", NormalType}
  //      {"cmdline", NormalType}
  //      {"cmdline_size", IntType}
  //      {"env", NormalType}
  //      {"env_count", IntType}
  //      {"env_size", IntType}
  //      {"cwd", NormalType}
  //      {"auid", IntType}
  //      {"uid", IntType}
  //      {"euid", IntType}
  //      {"gid", IntType}
  //      {"egid", IntType}
  //      {"owner_uid", IntType}
  //      {"owner_gid", IntType}
  //      {"atime", IntType}
  //      {"mtime", IntType}
  //      {"ctime", IntType}
  //      {"btime", IntType}
  //      {"overflows", NormalType}
  //      {"parent", IntType}
  //      {"time", IntType}
  //      {"uptime", IntType}
  //      {"eid", NormalType}
  //      {"status", IntType}
  //}
  // 4. Perform validation
  // validate_rows(data, row_map);
}

} // namespace osquery
