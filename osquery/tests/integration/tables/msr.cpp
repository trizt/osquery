
/**
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under both the Apache 2.0 license (found in the
 *  LICENSE file in the root directory of this source tree) and the GPLv2 (found
 *  in the COPYING file in the root directory of this source tree).
 *  You may select, at your option, one of the above-listed licenses.
 */

// Sanity check integration test for msr
// Spec file: specs/linux/msr.table

#include <osquery/tests/integration/tables/helper.h>

namespace osquery {

class msr : public IntegrationTableTest {};

TEST_F(msr, test_sanity) {
  // 1. Query data
  // QueryData data = execute_query("select * from msr");
  // 2. Check size before validation
  // ASSERT_GE(data.size(), 0ul);
  // ASSERT_EQ(data.size(), 1ul);
  // ASSERT_EQ(data.size(), 0ul);
  // 3. Build validation map
  // See IntegrationTableTest.cpp for avaialbe flags
  // Or use custom DataCheck object
  // ValidatatioMap row_map = {
  //      {"processor_number", IntType}
  //      {"turbo_disabled", IntType}
  //      {"turbo_ratio_limit", IntType}
  //      {"platform_info", IntType}
  //      {"perf_ctl", IntType}
  //      {"perf_status", IntType}
  //      {"feature_control", IntType}
  //      {"rapl_power_limit", IntType}
  //      {"rapl_energy_status", IntType}
  //      {"rapl_power_units", IntType}
  //}
  // 4. Perform validation
  // validate_rows(data, row_map);
}

} // namespace osquery
