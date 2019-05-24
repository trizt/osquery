
/**
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under both the Apache 2.0 license (found in the
 *  LICENSE file in the root directory of this source tree) and the GPLv2 (found
 *  in the COPYING file in the root directory of this source tree).
 *  You may select, at your option, one of the above-listed licenses.
 */

// Sanity check integration test for launchd
// Spec file: specs/darwin/launchd.table

#include <osquery/tests/integration/tables/helper.h>

namespace osquery {

class launchd : public IntegrationTableTest {};

TEST_F(launchd, test_sanity) {
  // 1. Query data
  // QueryData data = execute_query("select * from launchd");
  // 2. Check size before validation
  // ASSERT_GE(data.size(), 0ul);
  // ASSERT_EQ(data.size(), 1ul);
  // ASSERT_EQ(data.size(), 0ul);
  // 3. Build validation map
  // See IntegrationTableTest.cpp for avaialbe flags
  // Or use custom DataCheck object
  // ValidatatioMap row_map = {
  //      {"path", NormalType}
  //      {"name", NormalType}
  //      {"label", NormalType}
  //      {"program", NormalType}
  //      {"run_at_load", NormalType}
  //      {"keep_alive", NormalType}
  //      {"on_demand", NormalType}
  //      {"disabled", NormalType}
  //      {"username", NormalType}
  //      {"groupname", NormalType}
  //      {"stdout_path", NormalType}
  //      {"stderr_path", NormalType}
  //      {"start_interval", NormalType}
  //      {"program_arguments", NormalType}
  //      {"watch_paths", NormalType}
  //      {"queue_directories", NormalType}
  //      {"inetd_compatibility", NormalType}
  //      {"start_on_mount", NormalType}
  //      {"root_directory", NormalType}
  //      {"working_directory", NormalType}
  //      {"process_type", NormalType}
  //}
  // 4. Perform validation
  // EXPECT_TRUE(validate_rows(data, row_map));
}

} // namespace osquery
