
/**
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under both the Apache 2.0 license (found in the
 *  LICENSE file in the root directory of this source tree) and the GPLv2 (found
 *  in the COPYING file in the root directory of this source tree).
 *  You may select, at your option, one of the above-listed licenses.
 */

// Sanity check integration test for md_devices
// Spec file: specs/linux/md_devices.table

#include <osquery/tests/integration/tables/helper.h>

namespace osquery {

class mdDevices : public IntegrationTableTest {};

TEST_F(mdDevices, test_sanity) {
  // 1. Query data
  // QueryData data = execute_query("select * from md_devices");
  // 2. Check size before validation
  // ASSERT_GE(data.size(), 0ul);
  // ASSERT_EQ(data.size(), 1ul);
  // ASSERT_EQ(data.size(), 0ul);
  // 3. Build validation map
  // See IntegrationTableTest.cpp for avaialbe flags
  // Or use custom DataCheck object
  // ValidatatioMap row_map = {
  //      {"device_name", NormalType}
  //      {"status", NormalType}
  //      {"raid_level", IntType}
  //      {"size", IntType}
  //      {"chunk_size", IntType}
  //      {"raid_disks", IntType}
  //      {"nr_raid_disks", IntType}
  //      {"working_disks", IntType}
  //      {"active_disks", IntType}
  //      {"failed_disks", IntType}
  //      {"spare_disks", IntType}
  //      {"superblock_state", NormalType}
  //      {"superblock_version", NormalType}
  //      {"superblock_update_time", IntType}
  //      {"bitmap_on_mem", NormalType}
  //      {"bitmap_chunk_size", NormalType}
  //      {"bitmap_external_file", NormalType}
  //      {"recovery_progress", NormalType}
  //      {"recovery_finish", NormalType}
  //      {"recovery_speed", NormalType}
  //      {"resync_progress", NormalType}
  //      {"resync_finish", NormalType}
  //      {"resync_speed", NormalType}
  //      {"reshape_progress", NormalType}
  //      {"reshape_finish", NormalType}
  //      {"reshape_speed", NormalType}
  //      {"check_array_progress", NormalType}
  //      {"check_array_finish", NormalType}
  //      {"check_array_speed", NormalType}
  //      {"unused_devices", NormalType}
  //      {"other", NormalType}
  //}
  // 4. Perform validation
  // validate_rows(data, row_map);
}

} // namespace osquery
