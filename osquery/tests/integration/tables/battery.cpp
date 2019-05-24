
/**
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under both the Apache 2.0 license (found in the
 *  LICENSE file in the root directory of this source tree) and the GPLv2 (found
 *  in the COPYING file in the root directory of this source tree).
 *  You may select, at your option, one of the above-listed licenses.
 */

// Sanity check integration test for battery
// Spec file: specs/darwin/battery.table

#include <osquery/tests/integration/tables/helper.h>

namespace osquery {

class battery : public IntegrationTableTest {};

TEST_F(battery, test_sanity) {
  // 1. Query data
  // QueryData data = execute_query("select * from battery");
  // 2. Check size before validation
  // ASSERT_GE(data.size(), 0ul);
  // ASSERT_EQ(data.size(), 1ul);
  // ASSERT_EQ(data.size(), 0ul);
  // 3. Build validation map
  // See IntegrationTableTest.cpp for avaialbe flags
  // Or use custom DataCheck object
  // ValidatatioMap row_map = {
  //      {"manufacturer", NormalType}
  //      {"manufacture_date", IntType}
  //      {"model", NormalType}
  //      {"serial_number", NormalType}
  //      {"cycle_count", IntType}
  //      {"health", NormalType}
  //      {"condition", NormalType}
  //      {"state", NormalType}
  //      {"charging", IntType}
  //      {"charged", IntType}
  //      {"designed_capacity", IntType}
  //      {"max_capacity", IntType}
  //      {"current_capacity", IntType}
  //      {"percent_remaining", IntType}
  //      {"amperage", IntType}
  //      {"voltage", IntType}
  //      {"minutes_until_empty", IntType}
  //      {"minutes_to_full_charge", IntType}
  //}
  // 4. Perform validation
  // validate_rows(data, row_map);
}

} // namespace osquery
