
/**
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under both the Apache 2.0 license (found in the
 *  LICENSE file in the root directory of this source tree) and the GPLv2 (found
 *  in the COPYING file in the root directory of this source tree).
 *  You may select, at your option, one of the above-listed licenses.
 */

// Sanity check integration test for physical_disk_performance
// Spec file: specs/windows/physical_disk_performance.table

#include <osquery/tests/integration/tables/helper.h>

namespace osquery {

class physicalDiskPerformance : public IntegrationTableTest {};

TEST_F(physicalDiskPerformance, test_sanity) {
  // 1. Query data
  // QueryData data = execute_query("select * from physical_disk_performance");
  // 2. Check size before validation
  // ASSERT_GE(data.size(), 0ul);
  // ASSERT_EQ(data.size(), 1ul);
  // ASSERT_EQ(data.size(), 0ul);
  // 3. Build validation map
  // See IntegrationTableTest.cpp for avaialbe flags
  // Or use custom DataCheck object
  // ValidatatioMap row_map = {
  //      {"name", NormalType}
  //      {"avg_disk_bytes_per_read", IntType}
  //      {"avg_disk_bytes_per_write", IntType}
  //      {"avg_disk_read_queue_length", IntType}
  //      {"avg_disk_write_queue_length", IntType}
  //      {"avg_disk_sec_per_read", IntType}
  //      {"avg_disk_sec_per_write", IntType}
  //      {"current_disk_queue_length", IntType}
  //      {"percent_disk_read_time", IntType}
  //      {"percent_disk_write_time", IntType}
  //      {"percent_disk_time", IntType}
  //      {"percent_idle_time", IntType}
  //}
  // 4. Perform validation
  // EXPECT_TRUE(validate_rows(data, row_map));
}

} // namespace osquery
