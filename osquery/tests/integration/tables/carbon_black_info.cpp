
/**
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under both the Apache 2.0 license (found in the
 *  LICENSE file in the root directory of this source tree) and the GPLv2 (found
 *  in the COPYING file in the root directory of this source tree).
 *  You may select, at your option, one of the above-listed licenses.
 */

// Sanity check integration test for carbon_black_info
// Spec file: specs/carbon_black_info.table

#include <osquery/tests/integration/tables/helper.h>

namespace osquery {

class carbonBlackInfo : public IntegrationTableTest {};

TEST_F(carbonBlackInfo, test_sanity) {
  // 1. Query data
  // QueryData data = execute_query("select * from carbon_black_info");
  // 2. Check size before validation
  // ASSERT_GE(data.size(), 0ul);
  // ASSERT_EQ(data.size(), 1ul);
  // ASSERT_EQ(data.size(), 0ul);
  // 3. Build validation map
  // See IntegrationTableTest.cpp for avaialbe flags
  // Or use custom DataCheck object
  // ValidatatioMap row_map = {
  //      {"sensor_id", IntType}
  //      {"config_name", NormalType}
  //      {"collect_store_files", IntType}
  //      {"collect_module_loads", IntType}
  //      {"collect_module_info", IntType}
  //      {"collect_file_mods", IntType}
  //      {"collect_reg_mods", IntType}
  //      {"collect_net_conns", IntType}
  //      {"collect_processes", IntType}
  //      {"collect_cross_processes", IntType}
  //      {"collect_emet_events", IntType}
  //      {"collect_data_file_writes", IntType}
  //      {"collect_process_user_context", IntType}
  //      {"collect_sensor_operations", IntType}
  //      {"log_file_disk_quota_mb", IntType}
  //      {"log_file_disk_quota_percentage", IntType}
  //      {"protection_disabled", IntType}
  //      {"sensor_ip_addr", NormalType}
  //      {"sensor_backend_server", NormalType}
  //      {"event_queue", IntType}
  //      {"binary_queue", IntType}
  //}
  // 4. Perform validation
  // EXPECT_TRUE(validate_rows(data, row_map));
}

} // namespace osquery
